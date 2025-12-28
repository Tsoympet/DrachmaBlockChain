#include "contractcaller.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>

namespace {
QString erc20_interface()
{
    return R"([
        {"name":"name","type":"function","stateMutability":"view","inputs":[],"outputs":[{"name":"","type":"string"}]},
        {"name":"symbol","type":"function","stateMutability":"view","inputs":[],"outputs":[{"name":"","type":"string"}]},
        {"name":"decimals","type":"function","stateMutability":"view","inputs":[],"outputs":[{"name":"","type":"uint8"}]},
        {"name":"balanceOf","type":"function","stateMutability":"view","inputs":[{"name":"owner","type":"address"}],"outputs":[{"name":"","type":"uint256"}]},
        {"name":"transfer","type":"function","stateMutability":"nonpayable","inputs":[{"name":"to","type":"address"},{"name":"value","type":"uint256"}],"outputs":[{"name":"","type":"bool"}]}
    ])";
}

QString erc721_interface()
{
    return R"([
        {"name":"name","type":"function","stateMutability":"view","inputs":[],"outputs":[{"name":"","type":"string"}]},
        {"name":"symbol","type":"function","stateMutability":"view","inputs":[],"outputs":[{"name":"","type":"string"}]},
        {"name":"ownerOf","type":"function","stateMutability":"view","inputs":[{"name":"tokenId","type":"uint256"}],"outputs":[{"name":"","type":"address"}]},
        {"name":"transferFrom","type":"function","stateMutability":"nonpayable","inputs":[{"name":"from","type":"address"},{"name":"to","type":"address"},{"name":"tokenId","type":"uint256"}],"outputs":[]}
    ])";
}
}

ContractCaller::ContractCaller(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* v = new QVBoxLayout(this);
    QFormLayout* f = new QFormLayout();
    address_edit = new QLineEdit(this);
    address_edit->setPlaceholderText("0x...");
    interface_box = new QComboBox(this);
    interface_box->addItems({"Custom ABI", "ERC-20", "ERC-721"});
    abi_edit = new QTextEdit(this);
    abi_edit->setPlaceholderText("Paste ABI JSON here");
    abi_edit->setAcceptRichText(false);
    abi_edit->setMinimumHeight(120);

    helper_label = new QLabel("Functions will be parsed automatically.", this);
    function_tree = new QTreeWidget(this);
    function_tree->setHeaderLabels({"Function", "Type"});
    function_tree->setRootIsDecorated(false);

    QPushButton* parse_btn = new QPushButton("Parse ABI", this);
    QPushButton* call_btn = new QPushButton("Call (read)", this);
    QPushButton* send_btn = new QPushButton("Send transaction (write)", this);

    connect(parse_btn, &QPushButton::clicked, this, [this]{ apply_abi(abi_edit->toPlainText()); });
    connect(call_btn, &QPushButton::clicked, this, [this]{ handle_call(false); });
    connect(send_btn, &QPushButton::clicked, this, [this]{ handle_call(true); });
    connect(interface_box, &QComboBox::currentTextChanged, this, [this](const QString& text){
        if (text == "ERC-20") abi_edit->setPlainText(erc20_interface());
        else if (text == "ERC-721") abi_edit->setPlainText(erc721_interface());
    });

    f->addRow("Contract address", address_edit);
    f->addRow("ABI preset", interface_box);

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
    if (err.error != QJsonParseError::NoError || !doc.isArray()) {
        helper_label->setText(QString("ABI parse error: %1").arg(err.errorString()));
        function_tree->clear();
        return;
    }
    populate_functions(doc.array());
}

void ContractCaller::populate_functions(const QJsonArray& abi_array)
{
    function_tree->clear();
    for (const QJsonValue& v : abi_array) {
        if (!v.isObject()) continue;
        QJsonObject obj = v.toObject();
        QString type = obj.value("type").toString();
        if (type != "function") continue;
        QString name = obj.value("name").toString("<anonymous>");
        QString state = obj.value("stateMutability").toString();
        auto* item = new QTreeWidgetItem({name, state});
        function_tree->addTopLevelItem(item);
    }
    helper_label->setText("Parsed functions; select to call or send.");
    function_tree->resizeColumnToContents(0);
}

void ContractCaller::handle_call(bool write)
{
    if (address_edit->text().isEmpty()) {
        QMessageBox::warning(this, "Contract", "Please enter a contract address.");
        return;
    }
    emit call_requested(address_edit->text(), abi_edit->toPlainText(), write);
}

