#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QToolButton>
#include <QUrl>
#include <QListWidgetItem>

class QWebEngineView;

class DappBrowser : public QWidget {
    Q_OBJECT
public:
    explicit DappBrowser(QWidget* parent = nullptr);

    void set_default_url(const QUrl& url);
    void set_popular(const QList<QPair<QString, QUrl>>& items);

signals:
    void url_requested(const QUrl& url);

private slots:
    void open_url();
    void handle_shortcut(QListWidgetItem* item);

private:
    QLineEdit* url_edit{nullptr};
    QToolButton* go_btn{nullptr};
    QListWidget* shortcuts{nullptr};
    QWebEngineView* view{nullptr};
};

