#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

struct NftItem {
    QString token_id;
    QString name;
    QString image_url;
};

class NftGallery : public QWidget {
    Q_OBJECT
public:
    explicit NftGallery(QWidget* parent = nullptr);

    void set_items(const QList<NftItem>& items);

signals:
    void transfer_requested(const QString& token_id, const QString& to);
    void mint_requested();

private:
    void trigger_transfer();

    QTreeWidget* tree{nullptr};
    QPushButton* transfer_btn{nullptr};
    QPushButton* mint_btn{nullptr};
};

