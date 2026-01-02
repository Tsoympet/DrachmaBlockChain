#pragma once

#include <QWidget>
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QProgressBar>
#include <QTabWidget>

class ContractCaller;
class NftGallery;
class DappBrowser;

class SidechainView : public QWidget {
    Q_OBJECT
public:
    explicit SidechainView(QWidget* parent = nullptr);

    void set_balances(double tln_balance, double drm_balance, double obl_balance);
    void set_sidechain_status(const QString& status_text, int sync_progress, int peer_count);
    void apply_abi_json(const QString& json_text);

signals:
    void request_lock_to_sidechain();
    void request_burn_from_sidechain();
    void request_contract_call(const QString& address, const QString& abi, bool write);
    void request_nft_transfer(const QString& token_id, const QString& to);
    void request_nft_mint();
    void request_open_dapp(const QUrl& url);

private:
    QWidget* build_balances_widget();
    QWidget* build_bridge_widget();
    QWidget* build_contracts_widget();
    QWidget* build_nft_widget();
    QWidget* build_dapps_widget();

    void seed_mock_data();

    QLabel* tln_balance_label{nullptr};
    QLabel* drm_balance_label{nullptr};
    QLabel* obl_balance_label{nullptr};
    QLabel* status_label{nullptr};
    QProgressBar* sync_bar{nullptr};
    QLabel* peer_label{nullptr};

    ContractCaller* contract_caller{nullptr};
    NftGallery* nft_gallery{nullptr};
    DappBrowser* dapp_browser{nullptr};
};
