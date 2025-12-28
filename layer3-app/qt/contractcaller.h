#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

class ContractCaller : public QWidget {
    Q_OBJECT
public:
    explicit ContractCaller(QWidget* parent = nullptr);

    QString selected_address() const;
    QString abi_json() const;
    void apply_abi(const QString& text);

signals:
    void call_requested(const QString& address, const QString& abi, bool write);

private:
    void populate_functions(const QJsonArray& abi_array);
    void handle_call(bool write);

    QLineEdit* address_edit{nullptr};
    QComboBox* interface_box{nullptr};
    QTextEdit* abi_edit{nullptr};
    QTreeWidget* function_tree{nullptr};
    QLabel* helper_label{nullptr};
};

