#include "dappbrowser.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWebEngineView>
#include <QListWidgetItem>

DappBrowser::DappBrowser(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* v = new QVBoxLayout(this);
    QHBoxLayout* bar = new QHBoxLayout();
    url_edit = new QLineEdit(this);
    url_edit->setPlaceholderText("http://localhost:8080");
    go_btn = new QToolButton(this);
    go_btn->setText("Go");
    bar->addWidget(url_edit);
    bar->addWidget(go_btn);
    v->addLayout(bar);

    shortcuts = new QListWidget(this);
    shortcuts->setMaximumHeight(120);
    shortcuts->setToolTip("Common dApps");
    view = new QWebEngineView(this);

    v->addWidget(shortcuts);
    v->addWidget(view, 1);

    connect(go_btn, &QToolButton::clicked, this, &DappBrowser::open_url);
    connect(shortcuts, &QListWidget::itemClicked, this, &DappBrowser::handle_shortcut);
}

void DappBrowser::set_default_url(const QUrl& url)
{
    url_edit->setText(url.toString());
    view->setUrl(url);
}

void DappBrowser::set_popular(const QList<QPair<QString, QUrl>>& items)
{
    shortcuts->clear();
    for (const auto& pair : items) {
        auto* item = new QListWidgetItem(pair.first, shortcuts);
        item->setData(Qt::UserRole, pair.second);
    }
}

void DappBrowser::open_url()
{
    QUrl url = QUrl::fromUserInput(url_edit->text());
    if (!url.isValid()) return;
    view->setUrl(url);
    emit url_requested(url);
}

void DappBrowser::handle_shortcut(QListWidgetItem* item)
{
    QUrl url = item->data(Qt::UserRole).toUrl();
    if (!url.isValid()) return;
    url_edit->setText(url.toString());
    open_url();
}

