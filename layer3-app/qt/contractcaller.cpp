#include "contractcaller.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>

ContractCaller::ContractCaller(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* v = new QVBoxLayout(this);
    QFormLayout* f = new QFormLayout();
    address_edit = new QLineEdit(this);
    address_edit->setPlaceholderText("module-id (e.g. wasm.contract.greet)");
    interface_box = new QComboBox(this);
    interface_box->addItems({"Custom WASM manifest", "DRM contract (deploy/call)", "OBL dApp (call_dapp)"});
    abi_edit = new QTextEdit(this);
    abi_edit->setPlaceholderText("Paste WASM manifest JSON: {\"module\":\"<id>\",\"exports\":[\"init\",\"entry\"]}");
    abi_edit->setAcceptRichText(false);
    abi_edit->setMinimumHeight(120);

    helper_label = new QLabel("Exports will be parsed from the manifest.", this);
    function_tree = new QTreeWidget(this);
    function_tree->setHeaderLabels({"Export", "Domain"});
    function_tree->setRootIsDecorated(false);

    QPushButton* parse_btn = new QPushButton("Parse manifest", this);
    QPushButton* call_btn = new QPushButton("Call (read)", this);
    QPushButton* send_btn = new QPushButton("Execute (write)", this);

    connect(parse_btn, &QPushButton::clicked, this, [this]{ apply_abi(abi_edit->toPlainText()); });
    connect(call_btn, &QPushButton::clicked, this, [this]{ handle_call(false); });
    connect(send_btn, &QPushButton::clicked, this, [this]{ handle_call(true); });
    connect(interface_box, &QComboBox::currentTextChanged, this, [this](const QString& text){
        if (text == "DRM contract (deploy/call)") {
            abi_edit->setPlainText(R"({"module":"drm.contract.sample","exports":["init","handle_call"]})");
        } else if (text == "OBL dApp (call_dapp)") {
            abi_edit->setPlainText(R"({"module":"obl.dapp.sample","exports":["query","interaction"]})");
        }
    });

    f->addRow("Module ID", address_edit);
    f->addRow("Manifest preset", interface_box);

    v->addLayout(f);
    v->addWidget(abi_edit);
    v->addWidget(helper_label);
    v->addWidget(function_tree);

    QHBoxLayout* btns = new QHBoxLayout();
    btns->addWidget(parse_btn);
    btns->addStretch(1);
    btns->addWidget(call_btn);
    btns->addWidget(send_btn);
    v->addLayout(btns);
}

QString ContractCaller::selected_address() const
{
    return address_edit->text();
}

QString ContractCaller::abi_json() const
{
    return abi_edit->toPlainText();
}

void ContractCaller::apply_abi(const QString& text)
{
    abi_edit->setPlainText(text);
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        helper_label->setText(QString("Manifest parse error: %1").arg(err.errorString()));
        function_tree->clear();
        return;
    }
    const QJsonObject obj = doc.object();
    if (address_edit->text().isEmpty()) {
        const QString module = obj.value("module").toString();
        if (!module.isEmpty()) address_edit->setText(module);
    }
    const QJsonArray exports = obj.value("exports").toArray();
    populate_functions(exports);
}

void ContractCaller::populate_functions(const QJsonArray& exports)
{
    function_tree->clear();
    for (const QJsonValue& v : exports) {
        if (!v.isString()) continue;
        const QString name = v.toString("<export>");
        auto* item = new QTreeWidgetItem({name, "WASM"});
        function_tree->addTopLevelItem(item);
    }
    helper_label->setText("Parsed WASM exports; select to call or execute.");
    function_tree->resizeColumnToContents(0);
}

void ContractCaller::handle_call(bool write)
{
    if (address_edit->text().isEmpty()) {
        QMessageBox::warning(this, "Contract", "Please enter a module identifier.");
        return;
    }
    emit call_requested(address_edit->text(), abi_edit->toPlainText(), write);
}
