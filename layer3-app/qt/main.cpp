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
#include <atomic>
#include <thread>
#include <mutex>
#include <deque>
#include <random>
#include <chrono>
#include <sstream>

// Drachma Core GUI embeds a lightweight controller around the consensus and
// networking layers without implementing consensus logic directly. All
// consensus-critical rules remain in layer1-core; the GUI only orchestrates
// service lifecycles, user interactions, and status reporting.

struct WalletTx {
    QString txid;
    qint64 amount; // satoshis-like smallest unit
    QDateTime timestamp;
    QString direction;
};

class WalletModel : public QObject {
    Q_OBJECT
public:
    explicit WalletModel(QObject* parent = nullptr) : QObject(parent), m_balance(0) {}

    qint64 balance() const { return m_balance.load(); }

    QStringList addresses() const {
        std::lock_guard<std::mutex> g(m_mutex);
        return m_addresses;
    }

    const std::deque<WalletTx>& transactions() const { return m_txs; }

    QString generateAddress()
    {
        // Deterministic pseudo address for UI; actual key management is handled
        // by the wallet backend in layer2-services. Here we only provide a
        // user-facing identifier.
        auto now = QDateTime::currentDateTimeUtc();
        QString addr = QString("drm_%1").arg(now.toSecsSinceEpoch());
        {
            std::lock_guard<std::mutex> g(m_mutex);
            m_addresses.push_back(addr);
        }
        return addr;
    }

    void credit(qint64 amount, const QString& txid)
    {
        m_balance += amount;
        addTx({txid, amount, QDateTime::currentDateTimeUtc(), "receive"});
    }

    bool debit(qint64 amount, const QString& txid)
    {
        qint64 current = m_balance.load();
        if (amount <= 0 || current < amount)
            return false;
        m_balance -= amount;
        addTx({txid, -amount, QDateTime::currentDateTimeUtc(), "send"});
        return true;
    }

signals:
    void balanceChanged(qint64 newBalance);
    void transactionAdded(const WalletTx& tx);

private:
    void addTx(const WalletTx& tx)
    {
        {
            std::lock_guard<std::mutex> g(m_mutex);
            m_txs.push_front(tx);
            if (m_txs.size() > 1000)
                m_txs.pop_back();
        }
        emit balanceChanged(m_balance.load());
        emit transactionAdded(tx);
    }

    std::atomic<qint64> m_balance;
    QStringList m_addresses;
    std::deque<WalletTx> m_txs;
    mutable std::mutex m_mutex;
};

class NodeController : public QObject {
    Q_OBJECT
public:
    explicit NodeController(QObject* parent = nullptr)
        : QObject(parent), m_running(false), m_height(0), m_peers(0), m_hashrate(0.0) {}

    ~NodeController() { stop(); }

    void start()
    {
        if (m_running.exchange(true))
            return;
        m_worker = std::thread([this]{ this->loop(); });
    }

    void stop()
    {
        if (!m_running.exchange(false))
            return;
        if (m_worker.joinable()) m_worker.join();
    }

    int height() const { return m_height.load(); }
    int peers() const { return m_peers.load(); }
    double hashrate() const { return m_hashrate.load(); }

signals:
    void statusUpdated(int height, int peers, double hashrate);

private:
    void loop()
    {
        std::mt19937_64 rng{static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count())};
        std::uniform_int_distribution<int> peerChange(-1, 2);
        while (m_running.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            m_height += 1; // Simulate steady block sync for UI display only.
            int delta = peerChange(rng);
            int currentPeers = std::max(0, m_peers.load() + delta);
            m_peers.store(currentPeers);
            double syntheticHashrate = 50.0 + (m_height.load() % 10) * 5.0;
            m_hashrate.store(syntheticHashrate);
            emit statusUpdated(m_height.load(), currentPeers, syntheticHashrate);
        }
    }

    std::atomic<bool> m_running;
    std::atomic<int> m_height;
    std::atomic<int> m_peers;
    std::atomic<double> m_hashrate;
    std::thread m_worker;
};

class MiningController : public QObject {
    Q_OBJECT
public:
    explicit MiningController(NodeController* node, QObject* parent = nullptr)
        : QObject(parent), m_node(node), m_mining(false), m_targetRate(1.0) {}

    bool isMining() const { return m_mining.load(); }
    double targetRate() const { return m_targetRate.load(); }

public slots:
    void start()
    {
        if (m_mining.exchange(true)) return;
        m_thread = std::thread([this]{ this->loop(); });
    }

    void stop()
    {
        if (!m_mining.exchange(false)) return;
        if (m_thread.joinable()) m_thread.join();
    }

    void setTargetRate(double hps)
    {
        m_targetRate.store(hps);
    }

signals:
    void miningTick(double simulatedHashrate);

private:
    void loop()
    {
        while (m_mining.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            double base = m_node ? m_node->hashrate() : 0.0;
            double hr = base + m_targetRate.load();
            emit miningTick(hr);
        }
    }

    NodeController* m_node;
    std::atomic<bool> m_mining;
    std::atomic<double> m_targetRate;
    std::thread m_thread;
};

static QString loadAsset(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    QTextStream ts(&f);
    return ts.readAll();
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow()
    {
        setWindowTitle("Drachma Core");
        resize(1100, 700);

        QWidget* central = new QWidget(this);
        QVBoxLayout* layout = new QVBoxLayout(central);

        QTabWidget* tabs = new QTabWidget(central);
        layout->addWidget(tabs);

        walletModel = new WalletModel(this);
        node = new NodeController(this);
        miner = new MiningController(node, this);

        tabs->addTab(buildOverview(), "Overview");
        tabs->addTab(buildSend(), "Send");
        tabs->addTab(buildReceive(), "Receive");
        tabs->addTab(buildTransactions(), "Transactions");
        tabs->addTab(buildMining(), "Mining");
        tabs->addTab(buildSettings(), "Settings");
        tabs->addTab(buildConsole(), "RPC Console");

        setCentralWidget(central);

        connect(walletModel, &WalletModel::balanceChanged, this, &MainWindow::updateBalance);
        connect(walletModel, &WalletModel::transactionAdded, this, &MainWindow::appendTx);
        connect(node, &NodeController::statusUpdated, this, &MainWindow::updateNodeStatus);
        connect(miner, &MiningController::miningTick, this, &MainWindow::updateMiningRate);

        node->start();
        walletModel->generateAddress();
        walletModel->credit(50 * 100000000LL, "genesis-ui-credit");
    }

    ~MainWindow() override
    {
        miner->stop();
        node->stop();
    }

private:
    QWidget* buildOverview()
    {
        QWidget* w = new QWidget(this);
        QVBoxLayout* v = new QVBoxLayout(w);

        QGroupBox* statusBox = new QGroupBox("Node status", w);
        QFormLayout* statusForm = new QFormLayout(statusBox);
        heightLbl = new QLabel("0", statusBox);
        peersLbl = new QLabel("0", statusBox);
        hashrateLbl = new QLabel("0 H/s", statusBox);
        statusForm->addRow("Block height", heightLbl);
        statusForm->addRow("Peers", peersLbl);
        statusForm->addRow("Network hashrate", hashrateLbl);
        statusBox->setLayout(statusForm);

        QGroupBox* walletBox = new QGroupBox("Wallet", w);
        QFormLayout* walletForm = new QFormLayout(walletBox);
        balanceLbl = new QLabel("0 DRM", walletBox);
        walletForm->addRow("Balance", balanceLbl);
        walletBox->setLayout(walletForm);

        QString tagline = loadAsset("layer3-app/assets/branding/tagline.txt");
        QLabel* taglineLbl = new QLabel(tagline.isEmpty() ? "Drachma Core" : tagline, w);
        taglineLbl->setWordWrap(true);

        v->addWidget(taglineLbl);
        v->addWidget(statusBox);
        v->addWidget(walletBox);
        v->addStretch(1);
        return w;
    }

    QWidget* buildSend()
    {
        QWidget* w = new QWidget(this);
        QVBoxLayout* v = new QVBoxLayout(w);
        QFormLayout* f = new QFormLayout();

        sendAddress = new QLineEdit(w);
        sendAmount = new QLineEdit(w);
        QPushButton* sendBtn = new QPushButton("Send", w);

        connect(sendBtn, &QPushButton::clicked, this, &MainWindow::performSend);

        f->addRow("Destination", sendAddress);
        f->addRow("Amount (DRM)", sendAmount);

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

        QPushButton* genBtn = new QPushButton("Generate new address", w);
        connect(genBtn, &QPushButton::clicked, this, &MainWindow::generateAddress);

        v->addWidget(new QLabel("Receiving addresses", w));
        v->addWidget(addressList);
        v->addWidget(genBtn);
        v->addStretch(1);
        return w;
    }

    QWidget* buildTransactions()
    {
        QWidget* w = new QWidget(this);
        QVBoxLayout* v = new QVBoxLayout(w);
        txList = new QListWidget(w);
        v->addWidget(txList);
        return w;
    }

    QWidget* buildMining()
    {
        QWidget* w = new QWidget(this);
        QVBoxLayout* v = new QVBoxLayout(w);
        QHBoxLayout* controls = new QHBoxLayout();

        QPushButton* startBtn = new QPushButton("Start miner", w);
        QPushButton* stopBtn = new QPushButton("Stop miner", w);
        QSpinBox* rateBox = new QSpinBox(w);
        rateBox->setRange(1, 1000);
        rateBox->setValue(10);
        QLabel* rateLbl = new QLabel("Target H/s", w);
        miningProgress = new QProgressBar(w);
        miningProgress->setRange(0, 1000);
        miningProgress->setValue(0);

        connect(startBtn, &QPushButton::clicked, miner, &MiningController::start);
        connect(stopBtn, &QPushButton::clicked, miner, &MiningController::stop);
        connect(rateBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int v){ miner->setTargetRate(v); });

        controls->addWidget(startBtn);
        controls->addWidget(stopBtn);
        controls->addWidget(rateLbl);
        controls->addWidget(rateBox);
        v->addLayout(controls);
        v->addWidget(new QLabel("Simulated miner output", w));
        v->addWidget(miningProgress);
        v->addStretch(1);
        return w;
    }

    QWidget* buildSettings()
    {
        QWidget* w = new QWidget(this);
        QVBoxLayout* v = new QVBoxLayout(w);

        QLabel* appName = new QLabel(loadAsset("layer3-app/assets/branding/app_name.txt"), w);
        appName->setWordWrap(true);
        QLabel* eula = new QLabel(loadAsset("layer3-app/assets/legal/EULA.txt"), w);
        eula->setWordWrap(true);

        v->addWidget(new QLabel("Application", w));
        v->addWidget(appName);
        v->addWidget(new QLabel("EULA", w));
        v->addWidget(eula);
        v->addStretch(1);
        return w;
    }

    QWidget* buildConsole()
    {
        QWidget* w = new QWidget(this);
        QVBoxLayout* v = new QVBoxLayout(w);
        consoleOutput = new QTextEdit(w);
        consoleOutput->setReadOnly(true);
        consoleInput = new QLineEdit(w);
        QPushButton* exec = new QPushButton("Execute", w);

        connect(exec, &QPushButton::clicked, this, &MainWindow::executeRpc);
        connect(consoleInput, &QLineEdit::returnPressed, this, &MainWindow::executeRpc);

        v->addWidget(consoleOutput);
        v->addWidget(consoleInput);
        v->addWidget(exec);
        return w;
    }

private slots:
    void updateBalance(qint64 value)
    {
        double drm = static_cast<double>(value) / 100000000.0;
        balanceLbl->setText(QString::number(drm, 'f', 8) + " DRM");
    }

    void appendTx(const WalletTx& tx)
    {
        QString line = QString("%1 %2 %3 DRM")
            .arg(tx.timestamp.toString(Qt::ISODate))
            .arg(tx.direction)
            .arg(static_cast<double>(tx.amount) / 100000000.0, 0, 'f', 8);
        txList->insertItem(0, line);
    }

    void updateNodeStatus(int height, int peers, double hashrate)
    {
        heightLbl->setText(QString::number(height));
        peersLbl->setText(QString::number(peers));
        hashrateLbl->setText(QString::number(hashrate, 'f', 2) + " H/s");
    }

    void updateMiningRate(double hr)
    {
        int scaled = static_cast<int>(hr);
        miningProgress->setValue(std::min(scaled, miningProgress->maximum()));
    }

    void generateAddress()
    {
        QString addr = walletModel->generateAddress();
        addressList->addItem(addr);
    }

    void performSend()
    {
        bool ok = false;
        double drm = sendAmount->text().toDouble(&ok);
        if (!ok || drm <= 0) {
            consoleOutput->append("Invalid amount");
            return;
        }
        qint64 units = static_cast<qint64>(drm * 100000000.0);
        QString dest = sendAddress->text().trimmed();
        if (dest.isEmpty()) {
            consoleOutput->append("Destination required");
            return;
        }
        QString txid = QString("ui-%1").arg(QDateTime::currentMSecsSinceEpoch());
        if (walletModel->debit(units, txid)) {
            consoleOutput->append("Sent " + QString::number(drm, 'f', 8) + " DRM to " + dest);
        } else {
            consoleOutput->append("Insufficient balance");
        }
    }

    void executeRpc()
    {
        QString cmd = consoleInput->text().trimmed();
        if (cmd.isEmpty()) return;
        consoleInput->clear();
        if (cmd == "getblockchaininfo") {
            consoleOutput->append(QString("height=%1 peers=%2").arg(node->height()).arg(node->peers()));
        } else if (cmd == "getbalance") {
            consoleOutput->append(QString::number(static_cast<double>(walletModel->balance()) / 100000000.0, 'f', 8));
        } else if (cmd == "help") {
            consoleOutput->append("Supported commands: getblockchaininfo, getbalance");
        } else {
            consoleOutput->append("Unknown command");
        }
    }

private:
    WalletModel* walletModel{nullptr};
    NodeController* node{nullptr};
    MiningController* miner{nullptr};

    QLabel* balanceLbl{nullptr};
    QLabel* heightLbl{nullptr};
    QLabel* peersLbl{nullptr};
    QLabel* hashrateLbl{nullptr};
    QListWidget* addressList{nullptr};
    QListWidget* txList{nullptr};
    QLineEdit* sendAddress{nullptr};
    QLineEdit* sendAmount{nullptr};
    QProgressBar* miningProgress{nullptr};
    QTextEdit* consoleOutput{nullptr};
    QLineEdit* consoleInput{nullptr};
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}

#include "main.moc"
