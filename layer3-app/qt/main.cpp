#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QFormLayout>
#include <QListWidget>
#include <QSplitter>
#include <QTimer>
#include <QGroupBox>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QSpinBox>
#include <QProgressBar>
#include <QMessageBox>
#include <QInputDialog>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>
#include <QMenuBar>
#include <QStatusBar>
#include <QPainter>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QFrame>
#include <QMutex>
#include <QMutexLocker>
#include <QDoubleSpinBox>
#include <QtGlobal>
#include <random>
#include <atomic>
#include <cstdlib>
#include <thread>

// The UI intentionally avoids embedding any consensus or validation logic.
// It orchestrates node lifecycle and surfaces wallet state via service calls.

namespace {

class AssetLocator {
public:
    static QString basePath()
    {
        // Prefer the repository-relative path first.
        QString candidate = QDir::current().absoluteFilePath("layer3-app/assets");
        if (QDir(candidate).exists()) return candidate;
        // Fallback to application path.
        QString appDir = QCoreApplication::applicationDirPath();
        candidate = QDir(appDir).absoluteFilePath("../assets");
        return candidate;
    }

    static QString textAsset(const QString& relative)
    {
        QFile f(filePath(relative));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
        QTextStream ts(&f);
        return ts.readAll();
    }

    static QString filePath(const QString& relative)
    {
        return QDir(basePath()).absoluteFilePath(relative);
    }
};

struct TransactionRow {
    QString txid;
    QString direction; // "send" or "receive"
    qint64 amount;     // in satoshis-like units
    int confirmations;
    QString status;    // pending/confirmed
    QDateTime timestamp;
};

class WalletServiceClient : public QObject {
    Q_OBJECT
public:
    explicit WalletServiceClient(QObject* parent = nullptr)
        : QObject(parent)
    {
        m_confirmed.store(0);
        m_unconfirmed.store(0);
        m_encrypted.store(false);
        m_locked.store(false);
        m_timer.setInterval(1000);
        connect(&m_timer, &QTimer::timeout, this, &WalletServiceClient::advanceConfirmations);
        m_timer.start();
    }

    qint64 confirmedBalance() const { return m_confirmed.load(); }
    qint64 unconfirmedBalance() const { return m_unconfirmed.load(); }

    bool isEncrypted() const { return m_encrypted.load(); }
    bool isLocked() const { return m_locked.load(); }

    QStringList addresses() const
    {
        QMutexLocker locker(&m_mutex);
        return m_addresses;
    }

    QList<TransactionRow> transactions() const
    {
        QMutexLocker locker(&m_mutex);
        return m_txs;
    }

    QString requestNewAddress(const QString& label)
    {
        // Addresses are deterministic placeholders; the backend owns key generation.
        QString addr = QString("drm_%1_%2").arg(QDateTime::currentMSecsSinceEpoch()).arg(m_addresses.size());
        {
            QMutexLocker locker(&m_mutex);
            m_addresses.append(addr + (label.isEmpty() ? "" : (" (" + label + ")")));
        }
        emit addressAdded(addr);
        return addr;
    }

    void creditSimulated(qint64 amount, const QString& memo)
    {
        TransactionRow row;
        row.txid = memo;
        row.direction = "receive";
        row.amount = amount;
        row.confirmations = 0;
        row.status = "pending";
        row.timestamp = QDateTime::currentDateTimeUtc();
        {
            QMutexLocker locker(&m_mutex);
            m_txs.prepend(row);
        }
        m_unconfirmed += amount;
        emit balancesChanged(m_confirmed.load(), m_unconfirmed.load());
        emit transactionsChanged();
    }

    bool createSpend(const QString& dest, qint64 amount, qint64 fee, QString& err)
    {
        if (m_locked.load()) {
            err = "Wallet is locked";
            return false;
        }
        qint64 total = amount + fee;
        if (total <= 0) {
            err = "Amount must be positive";
            return false;
        }
        if (m_confirmed.load() < total) {
            err = "Insufficient confirmed balance";
            return false;
        }
        TransactionRow row;
        row.txid = QString("tx_%1").arg(QDateTime::currentMSecsSinceEpoch());
        row.direction = "send";
        row.amount = -total;
        row.confirmations = 0;
        row.status = "pending";
        row.timestamp = QDateTime::currentDateTimeUtc();
        {
            QMutexLocker locker(&m_mutex);
            m_txs.prepend(row);
        }
        m_confirmed -= total;
        emit balancesChanged(m_confirmed.load(), m_unconfirmed.load());
        emit transactionsChanged();
        Q_UNUSED(dest);
        return true;
    }

    bool encryptWallet(const QString& passphrase, QString& err)
    {
        if (passphrase.size() < 8) {
            err = "Passphrase must be at least 8 characters";
            return false;
        }
        if (m_encrypted.load()) {
            err = "Wallet already encrypted";
            return false;
        }
        m_encrypted.store(true);
        m_locked.store(true);
        m_passphrase = passphrase;
        emit encryptionStateChanged();
        return true;
    }

    bool unlockWallet(const QString& passphrase, QString& err)
    {
        if (!m_encrypted.load()) {
            err = "Wallet is not encrypted";
            return false;
        }
        if (passphrase != m_passphrase) {
            err = "Incorrect passphrase";
            return false;
        }
        m_locked.store(false);
        emit encryptionStateChanged();
        return true;
    }

signals:
    void balancesChanged(qint64 confirmed, qint64 unconfirmed);
    void transactionsChanged();
    void addressAdded(const QString& addr);
    void encryptionStateChanged();

private slots:
    void advanceConfirmations()
    {
        QList<TransactionRow> snapshot;
        {
            QMutexLocker locker(&m_mutex);
            snapshot = m_txs;
        }
        bool changed = false;
        for (TransactionRow& row : snapshot) {
            if (row.confirmations < 6) {
                row.confirmations += 1;
                if (row.confirmations >= 2 && row.status == "pending") {
                    row.status = "confirmed";
                    if (row.amount > 0) {
                        m_unconfirmed -= row.amount;
                        m_confirmed += row.amount;
                    }
                }
                changed = true;
            }
        }
        if (changed) {
            QMutexLocker locker(&m_mutex);
            m_txs = snapshot;
            emit balancesChanged(m_confirmed.load(), m_unconfirmed.load());
            emit transactionsChanged();
        }
    }

private:
    mutable QMutex m_mutex;
    QStringList m_addresses;
    QList<TransactionRow> m_txs;
    std::atomic<qint64> m_confirmed;
    std::atomic<qint64> m_unconfirmed;
    std::atomic<bool> m_encrypted;
    std::atomic<bool> m_locked;
    QString m_passphrase;
    QTimer m_timer;
};

struct NodeConfig {
    QString dataDir;
    QString network; // mainnet / testnet
    QString rpcUser;
    QString rpcPass;
};

class NodeProcessController : public QObject {
    Q_OBJECT
public:
    explicit NodeProcessController(QObject* parent = nullptr)
        : QObject(parent)
    {
        connect(&m_pollTimer, &QTimer::timeout, this, &NodeProcessController::tick);
        m_pollTimer.setInterval(1000);
    }

    void startNode(const NodeConfig& cfg)
    {
        m_config = cfg;
        if (m_running.exchange(true)) return;
        m_height.store(0);
        m_syncProgress.store(0.0);
        m_peerCount.store(0);
        m_networkHashrate.store(0.0);
        m_pollTimer.start();
        emit nodeStarted();
    }

    void stopNode()
    {
        if (!m_running.exchange(false)) return;
        m_pollTimer.stop();
        emit nodeStopped();
    }

    bool isRunning() const { return m_running.load(); }

    struct RpcResult {
        bool ok{false};
        QString result;
        QString error;
    };

    RpcResult call(const QString& user, const QString& pass, const QString& method, const QString& params)
    {
        RpcResult res;
        if (!m_running.load()) {
            res.error = "Node offline";
            return res;
        }
        if (user != m_config.rpcUser || pass != m_config.rpcPass) {
            res.error = "Unauthorized";
            return res;
        }
        if (method == "ping") {
            res.ok = true; res.result = "pong"; return res;
        }
        if (method == "getblockchaininfo") {
            res.ok = true;
            res.result = QString("height=%1 peers=%2").arg(m_height.load()).arg(m_peerCount.load());
            return res;
        }
        res.error = QString("Unknown method %1").arg(method);
        return res;
    }

signals:
    void nodeStarted();
    void nodeStopped();
    void nodeStatusChanged(int height, double syncProgress, int peers, double networkHashrate);

private slots:
    void tick()
    {
        if (!m_running.load()) return;
        m_height.fetch_add(1);
        double progress = std::min(1.0, m_syncProgress.load() + 0.01);
        m_syncProgress.store(progress);
        int peers = qBound(0, m_peerCount.load() + (rand() % 3 - 1), 16);
        m_peerCount.store(peers);
        double nh = 75.0 + (m_height.load() % 30) * 2.5;
        m_networkHashrate.store(nh);
        emit nodeStatusChanged(m_height.load(), progress, peers, nh);
    }

private:
    std::atomic<bool> m_running{false};
    std::atomic<int> m_height{0};
    std::atomic<double> m_syncProgress{0.0};
    std::atomic<int> m_peerCount{0};
    std::atomic<double> m_networkHashrate{0.0};
    QTimer m_pollTimer;
    NodeConfig m_config;
};

class MiningManager : public QObject {
    Q_OBJECT
public:
    explicit MiningManager(NodeProcessController* node, QObject* parent = nullptr)
        : QObject(parent), m_node(node)
    {
        connect(&m_tick, &QTimer::timeout, this, &MiningManager::updateRate);
        m_tick.setInterval(1000);
    }

    void start(int threads, bool gpu)
    {
        m_threads = threads;
        m_gpuEnabled = gpu;
        if (m_running.exchange(true)) return;
        m_tick.start();
        emit miningStarted();
    }

    void stop()
    {
        if (!m_running.exchange(false)) return;
        m_tick.stop();
        emit miningStopped();
    }

    bool isRunning() const { return m_running.load(); }
    double lastHashrate() const { return m_lastHashrate.load(); }

signals:
    void hashrateUpdated(double rate);
    void miningStarted();
    void miningStopped();

private slots:
    void updateRate()
    {
        if (!m_running.load()) return;
        double base = 0.0;
        if (m_node) {
            base = m_node->isRunning() ? 15.0 : 5.0;
        }
        double simulated = base + m_threads * 12.5 + (m_gpuEnabled ? 150.0 : 0.0);
        m_lastHashrate.store(simulated);
        emit hashrateUpdated(simulated);
    }

private:
    NodeProcessController* m_node{nullptr};
    std::atomic<bool> m_running{false};
    std::atomic<double> m_lastHashrate{0.0};
    int m_threads{1};
    bool m_gpuEnabled{false};
    QTimer m_tick;
};

class EulaDialog : public QDialog {
    Q_OBJECT
public:
    explicit EulaDialog(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("DRACHMA End User License Agreement");
        QVBoxLayout* v = new QVBoxLayout(this);
        QTextEdit* text = new QTextEdit(this);
        text->setReadOnly(true);
        text->setPlainText(AssetLocator::textAsset("legal/EULA.txt"));
        v->addWidget(text);
        QCheckBox* accept = new QCheckBox("I have read and accept the EULA", this);
        v->addWidget(accept);
        QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        buttons->button(QDialogButtonBox::Ok)->setText("Accept");
        v->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, [this, accept]{
            if (!accept->isChecked()) {
                QMessageBox::warning(this, "EULA", "You must accept the EULA to continue.");
                return;
            }
            acceptState = true;
            accept();
        });
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        resize(640, 480);
    }

    bool accepted() const { return acceptState; }

private:
    bool acceptState{false};
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow()
    {
        setWindowTitle("DRACHMA Core Desktop");
        setMinimumSize(1200, 800);
        setWindowIcon(QIcon(AssetLocator::filePath("branding/app_icon.png")));

        createMenu();

        wallet = new WalletServiceClient(this);
        node = new NodeProcessController(this);
        miner = new MiningManager(node, this);

        QWidget* central = new QWidget(this);
        QVBoxLayout* layout = new QVBoxLayout(central);
        tabs = new QTabWidget(central);
        layout->addWidget(tabs);
        setCentralWidget(central);

        tabs->addTab(buildOverview(), "Overview");
        tabs->addTab(buildSend(), "Send");
        tabs->addTab(buildReceive(), "Receive");
        tabs->addTab(buildTransactions(), "Transactions");
        tabs->addTab(buildMining(), "Mining");
        tabs->addTab(buildSettings(), "Settings");

        connect(wallet, &WalletServiceClient::balancesChanged, this, &MainWindow::updateBalances);
        connect(wallet, &WalletServiceClient::transactionsChanged, this, &MainWindow::refreshTransactions);
        connect(wallet, &WalletServiceClient::addressAdded, this, &MainWindow::addAddress);
        connect(wallet, &WalletServiceClient::encryptionStateChanged, this, &MainWindow::updateEncryptionState);
        connect(node, &NodeProcessController::nodeStatusChanged, this, &MainWindow::updateNodeStatus);
        connect(node, &NodeProcessController::nodeStarted, this, [this]{ statusBar()->showMessage("Node started"); });
        connect(node, &NodeProcessController::nodeStopped, this, [this]{ statusBar()->showMessage("Node stopped"); });
        connect(miner, &MiningManager::hashrateUpdated, this, &MainWindow::updateHashrate);
        updateEncryptionState();

        // Load persisted settings and start the node.
        loadSettings();
        enforceEula();
        startNodeFromSettings();
        seedWallet();
        refreshTransactions();
    }

    ~MainWindow() override
    {
        miner->stop();
        node->stopNode();
    }

private:
    void createMenu()
    {
        QMenu* fileMenu = menuBar()->addMenu("File");
        QAction* eulaAction = fileMenu->addAction("View EULA");
        QAction* whitepaperAction = fileMenu->addAction("Open Whitepaper");
        QAction* exitAction = fileMenu->addAction("Exit");
        connect(eulaAction, &QAction::triggered, this, &MainWindow::showEulaDialog);
        connect(whitepaperAction, &QAction::triggered, this, &MainWindow::openWhitepaper);
        connect(exitAction, &QAction::triggered, this, &QWidget::close);
    }

    QWidget* buildOverview()
    {
        QWidget* w = new QWidget(this);
        QVBoxLayout* v = new QVBoxLayout(w);

        QLabel* tagline = new QLabel(AssetLocator::textAsset("branding/tagline.txt"), w);
        tagline->setWordWrap(true);
        v->addWidget(tagline);

        QGroupBox* nodeBox = new QGroupBox("Node sync status", w);
        QFormLayout* nf = new QFormLayout(nodeBox);
        heightLbl = new QLabel("0", nodeBox);
        peersLbl = new QLabel("0", nodeBox);
        syncLbl = new QLabel("0%", nodeBox);
        networkLbl = new QLabel("Offline", nodeBox);
        nf->addRow("Current height", heightLbl);
        nf->addRow("Peers", peersLbl);
        nf->addRow("Sync", syncLbl);
        nf->addRow("Network status", networkLbl);
        nodeBox->setLayout(nf);

        QGroupBox* walletBox = new QGroupBox("Wallet", w);
        QFormLayout* wf = new QFormLayout(walletBox);
        confirmedLbl = new QLabel("0 DRM", walletBox);
        unconfirmedLbl = new QLabel("0 DRM", walletBox);
        wf->addRow("Confirmed", confirmedLbl);
        wf->addRow("Unconfirmed", unconfirmedLbl);
        walletBox->setLayout(wf);

        v->addWidget(nodeBox);
        v->addWidget(walletBox);
        v->addStretch(1);
        return w;
    }

    QWidget* buildSend()
    {
        QWidget* w = new QWidget(this);
        QVBoxLayout* v = new QVBoxLayout(w);
        QFormLayout* f = new QFormLayout();

        destEdit = new QLineEdit(w);
        amountEdit = new QDoubleSpinBox(w);
        amountEdit->setRange(0.00000001, 21000000.0);
        amountEdit->setDecimals(8);
        feeBox = new QComboBox(w);
        feeBox->addItems({"Slow (0.0001)", "Normal (0.001)", "Fast (0.01)"});

        QPushButton* sendBtn = new QPushButton("Send", w);
        connect(sendBtn, &QPushButton::clicked, this, &MainWindow::confirmAndSend);

        f->addRow("Destination address", destEdit);
        f->addRow("Amount", amountEdit);
        f->addRow("Fee", feeBox);
        v->addLayout(f);
        v->addWidget(sendBtn);
        v->addStretch(1);
        return w;
    }

    QWidget* buildReceive()
    {
        QWidget* w = new QWidget(this);
        QVBoxLayout* v = new QVBoxLayout(w);
        addressList = new QListWidget(w);
        QLabel* qrLabelTitle = new QLabel("QR code (placeholder)", w);
        qrLabel = new QLabel(w);
        qrLabel->setFixedSize(220, 220);
        qrLabel->setFrameShape(QFrame::Box);

        QPushButton* genBtn = new QPushButton("Generate address", w);
        connect(genBtn, &QPushButton::clicked, this, &MainWindow::generateAddress);

        v->addWidget(new QLabel("Receiving addresses", w));
        v->addWidget(addressList);
        v->addWidget(qrLabelTitle);
        v->addWidget(qrLabel);
        v->addWidget(genBtn);
        v->addStretch(1);
        return w;
    }

    QWidget* buildTransactions()
    {
        QWidget* w = new QWidget(this);
        QVBoxLayout* v = new QVBoxLayout(w);
        txTable = new QTableWidget(w);
        txTable->setColumnCount(5);
        txTable->setHorizontalHeaderLabels({"Time", "TXID", "Direction", "Amount", "Confirmations"});
        txTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        txTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        txTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        v->addWidget(txTable);
        return w;
    }

    QWidget* buildMining()
    {
        QWidget* w = new QWidget(this);
        QVBoxLayout* v = new QVBoxLayout(w);
        QHBoxLayout* controls = new QHBoxLayout();
        QPushButton* startBtn = new QPushButton("Start mining", w);
        QPushButton* stopBtn = new QPushButton("Stop", w);
        cpuThreads = new QSpinBox(w);
        cpuThreads->setRange(1, std::thread::hardware_concurrency() == 0 ? 8 : static_cast<int>(std::thread::hardware_concurrency()));
        gpuToggle = new QCheckBox("Enable GPU (if available)", w);
        hashrateLbl = new QLabel("0 H/s", w);

        connect(startBtn, &QPushButton::clicked, this, &MainWindow::startMining);
        connect(stopBtn, &QPushButton::clicked, this, &MainWindow::stopMining);

        controls->addWidget(new QLabel("CPU threads", w));
        controls->addWidget(cpuThreads);
        controls->addWidget(gpuToggle);
        controls->addWidget(startBtn);
        controls->addWidget(stopBtn);
        v->addLayout(controls);

        v->addWidget(new QLabel("Hashrate", w));
        v->addWidget(hashrateLbl);
        return w;
    }

    QWidget* buildSettings()
    {
        QWidget* w = new QWidget(this);
        QVBoxLayout* v = new QVBoxLayout(w);
        QFormLayout* f = new QFormLayout();

        dataDirEdit = new QLineEdit(w);
        QPushButton* browse = new QPushButton("Browse", w);
        connect(browse, &QPushButton::clicked, this, &MainWindow::selectDataDir);

        QWidget* dirWidget = new QWidget(w);
        QHBoxLayout* dirLayout = new QHBoxLayout(dirWidget);
        dirLayout->setContentsMargins(0,0,0,0);
        dirLayout->addWidget(dataDirEdit);
        dirLayout->addWidget(browse);

        networkBox = new QComboBox(w);
        networkBox->addItems({"mainnet", "testnet"});
        rpcUserEdit = new QLineEdit(w);
        rpcPassEdit = new QLineEdit(w);
        rpcPassEdit->setEchoMode(QLineEdit::Password);

        encryptBtn = new QPushButton("Encrypt wallet", w);
        unlockBtn = new QPushButton("Unlock wallet", w);
        connect(encryptBtn, &QPushButton::clicked, this, &MainWindow::encryptWallet);
        connect(unlockBtn, &QPushButton::clicked, this, &MainWindow::unlockWallet);

        f->addRow("Data directory", dirWidget);
        f->addRow("Network", networkBox);
        f->addRow("RPC username", rpcUserEdit);
        f->addRow("RPC password", rpcPassEdit);
        f->addRow("Wallet encryption", encryptBtn);
        f->addRow("Wallet unlock", unlockBtn);

        QPushButton* save = new QPushButton("Save settings", w);
        connect(save, &QPushButton::clicked, this, &MainWindow::saveSettings);

        v->addLayout(f);
        v->addWidget(save);
        v->addStretch(1);
        return w;
    }

    void enforceEula()
    {
        QSettings s("Drachma", "CoreDesktop");
        if (s.value("eulaAccepted", false).toBool()) return;
        EulaDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted && dlg.accepted()) {
            s.setValue("eulaAccepted", true);
        } else {
            QMessageBox::critical(this, "EULA", "You must accept the EULA to use DRACHMA Core.");
            QTimer::singleShot(0, this, &QWidget::close);
        }
    }

    void seedWallet()
    {
        wallet->creditSimulated(25 * 100000000LL, "initial-aid");
        wallet->requestNewAddress("default");
    }

private slots:
    void updateBalances(qint64 confirmed, qint64 unconfirmed)
    {
        confirmedLbl->setText(QString::number(confirmed / 100000000.0, 'f', 8) + " DRM");
        unconfirmedLbl->setText(QString::number(unconfirmed / 100000000.0, 'f', 8) + " DRM");
    }

    void updateNodeStatus(int height, double syncProgress, int peers, double networkHashrate)
    {
        heightLbl->setText(QString::number(height));
        peersLbl->setText(QString::number(peers));
        syncLbl->setText(QString::number(syncProgress * 100.0, 'f', 2) + "%");
        networkLbl->setText(QString::number(networkHashrate, 'f', 2) + " H/s");
    }

    void updateHashrate(double rate)
    {
        hashrateLbl->setText(QString::number(rate, 'f', 2) + " H/s");
    }

    void refreshTransactions()
    {
        auto rows = wallet->transactions();
        txTable->setRowCount(rows.size());
        int i = 0;
        for (const auto& row : rows) {
            txTable->setItem(i, 0, new QTableWidgetItem(row.timestamp.toString(Qt::ISODate)));
            txTable->setItem(i, 1, new QTableWidgetItem(row.txid));
            txTable->setItem(i, 2, new QTableWidgetItem(row.direction));
            txTable->setItem(i, 3, new QTableWidgetItem(QString::number(row.amount / 100000000.0, 'f', 8)));
            txTable->setItem(i, 4, new QTableWidgetItem(QString::number(row.confirmations)));
            ++i;
        }
    }

    void addAddress(const QString& addr)
    {
        addressList->addItem(addr);
        drawQr(addr);
    }

    void generateAddress()
    {
        bool ok = false;
        QString label = QInputDialog::getText(this, "New address", "Label", QLineEdit::Normal, "", &ok);
        if (!ok) return;
        QString addr = wallet->requestNewAddress(label);
        drawQr(addr);
    }

    void drawQr(const QString& data)
    {
        QImage img(qrLabel->size(), QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setPen(Qt::black);
        p.drawRect(img.rect().adjusted(0,0,-1,-1));
        p.drawText(img.rect(), Qt::AlignCenter, data);
        p.end();
        qrLabel->setPixmap(QPixmap::fromImage(img));
    }

    void confirmAndSend()
    {
        QString dest = destEdit->text().trimmed();
        double amountD = amountEdit->value();
        qint64 amount = static_cast<qint64>(amountD * 100000000.0);
        qint64 fee = 0;
        switch (feeBox->currentIndex()) {
            case 0: fee = 10000; break;
            case 1: fee = 100000; break;
            case 2: fee = 1000000; break;
        }
        if (dest.isEmpty()) {
            QMessageBox::warning(this, "Send", "Destination address is required.");
            return;
        }
        QString summary = QString("Send %1 DRM to %2 with fee %3?")
            .arg(amountD, 0, 'f', 8)
            .arg(dest)
            .arg(fee / 100000000.0, 0, 'f', 8);
        if (QMessageBox::question(this, "Confirm transaction", summary) != QMessageBox::Yes)
            return;
        QString err;
        if (!wallet->createSpend(dest, amount, fee, err)) {
            QMessageBox::critical(this, "Send", err);
            return;
        }
        QMessageBox::information(this, "Send", "Transaction created and queued for broadcast.");
    }

    void startMining()
    {
        miner->start(cpuThreads->value(), gpuToggle->isChecked());
    }

    void stopMining()
    {
        miner->stop();
    }

    void selectDataDir()
    {
        QString dir = QFileDialog::getExistingDirectory(this, "Select data directory", dataDirEdit->text());
        if (!dir.isEmpty()) dataDirEdit->setText(dir);
    }

    void saveSettings()
    {
        QSettings s("Drachma", "CoreDesktop");
        s.setValue("dataDir", dataDirEdit->text());
        s.setValue("network", networkBox->currentText());
        s.setValue("rpcUser", rpcUserEdit->text());
        s.setValue("rpcPass", rpcPassEdit->text());
        statusBar()->showMessage("Settings saved", 3000);
        startNodeFromSettings();
    }

    void loadSettings()
    {
        QSettings s("Drachma", "CoreDesktop");
        QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        dataDirEdit->setText(s.value("dataDir", defaultDir).toString());
        networkBox->setCurrentText(s.value("network", "testnet").toString());
        rpcUserEdit->setText(s.value("rpcUser", "user").toString());
        rpcPassEdit->setText(s.value("rpcPass", "pass").toString());
    }

    void startNodeFromSettings()
    {
        NodeConfig cfg{dataDirEdit->text(), networkBox->currentText(), rpcUserEdit->text(), rpcPassEdit->text()};
        if (node->isRunning()) node->stopNode();
        node->startNode(cfg);
    }

    void encryptWallet()
    {
        bool ok = false;
        QString pass = QInputDialog::getText(this, "Encrypt wallet", "Passphrase", QLineEdit::Password, "", &ok);
        if (!ok) return;
        QString err;
        if (!wallet->encryptWallet(pass, err)) {
            QMessageBox::critical(this, "Encryption", err);
            return;
        }
        QMessageBox::information(this, "Encryption", "Wallet encrypted. Please keep your passphrase safe.");
    }

    void unlockWallet()
    {
        bool ok = false;
        QString pass = QInputDialog::getText(this, "Unlock wallet", "Passphrase", QLineEdit::Password, "", &ok);
        if (!ok) return;
        QString err;
        if (!wallet->unlockWallet(pass, err)) {
            QMessageBox::critical(this, "Unlock", err);
            return;
        }
        QMessageBox::information(this, "Unlock", "Wallet unlocked.");
    }

    void updateEncryptionState()
    {
        encryptBtn->setEnabled(!wallet->isEncrypted());
        unlockBtn->setEnabled(wallet->isEncrypted());
    }

    void showEulaDialog()
    {
        EulaDialog dlg(this);
        dlg.exec();
    }

    void openWhitepaper()
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(AssetLocator::filePath("docs/whitepaper.pdf")));
    }

private:
    WalletServiceClient* wallet{nullptr};
    NodeProcessController* node{nullptr};
    MiningManager* miner{nullptr};

    QTabWidget* tabs{nullptr};

    QLabel* heightLbl{nullptr};
    QLabel* peersLbl{nullptr};
    QLabel* syncLbl{nullptr};
    QLabel* networkLbl{nullptr};
    QLabel* confirmedLbl{nullptr};
    QLabel* unconfirmedLbl{nullptr};
    QLabel* hashrateLbl{nullptr};

    QLineEdit* destEdit{nullptr};
    QDoubleSpinBox* amountEdit{nullptr};
    QComboBox* feeBox{nullptr};

    QListWidget* addressList{nullptr};
    QLabel* qrLabel{nullptr};

    QTableWidget* txTable{nullptr};

    QSpinBox* cpuThreads{nullptr};
    QCheckBox* gpuToggle{nullptr};

    QLineEdit* dataDirEdit{nullptr};
    QComboBox* networkBox{nullptr};
    QLineEdit* rpcUserEdit{nullptr};
    QLineEdit* rpcPassEdit{nullptr};
    QPushButton* encryptBtn{nullptr};
    QPushButton* unlockBtn{nullptr};
};

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}

#include "main.moc"
