#include "nftgallery.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QInputDialog>

NftGallery::NftGallery(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* v = new QVBoxLayout(this);
    tree = new QTreeWidget(this);
    tree->setHeaderLabels({"Token ID", "Name", "Image URL"});
    tree->setRootIsDecorated(false);
    transfer_btn = new QPushButton("Transfer", this);
    mint_btn = new QPushButton("Mint (if allowed)", this);

    v->addWidget(tree);
    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addWidget(transfer_btn);
    buttons->addWidget(mint_btn);
    buttons->addStretch(1);
    v->addLayout(buttons);

    connect(transfer_btn, &QPushButton::clicked, this, &NftGallery::trigger_transfer);
    connect(mint_btn, &QPushButton::clicked, this, &NftGallery::mint_requested);
}

void NftGallery::set_items(const QList<NftItem>& items)
{
    tree->clear();
    for (const auto& item : items) {
        auto* row = new QTreeWidgetItem({item.token_id, item.name, item.image_url});
        tree->addTopLevelItem(row);
    }
    tree->resizeColumnToContents(0);
}

void NftGallery::trigger_transfer()
{
    auto* current = tree->currentItem();
    if (!current) {
        QMessageBox::warning(this, "NFT", "Select an NFT to transfer.");
        return;
    }
    bool ok = false;
    QString dest = QInputDialog::getText(this, "Transfer NFT", "Recipient address", QLineEdit::Normal, "0x...", &ok);
    if (ok && !dest.isEmpty()) {
        emit transfer_requested(current->text(0), dest);
    }
}

