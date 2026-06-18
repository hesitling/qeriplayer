/// @file NeteaseCaptchaLogin.cpp
/// @brief Interactive CLI tool for testing Netease captcha login flow
///
/// Usage:
///   ./NeteaseCaptchaLogin [--phone=PHONE] [--ctcode=86]
///
/// If --phone is omitted, you will be prompted interactively.

#include "api/netease/NeteaseClient.h"
#include "api/netease/NeteaseCrypto.h"
#include "core/network/HttpClient.h"

#include <QCoreApplication>
#include <QCoroTask>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>
#include <QUrl>

#include <iostream>
#include <string>

using namespace QeriPlayerQt;

static void printJson(const QJsonObject &obj, int indent = 0)
{
    QString prefix(indent * 2, QLatin1Char(' '));
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        if (it.value().isObject()) {
            std::cout << qPrintable(prefix) << qPrintable(it.key()) << ":\n";
            printJson(it.value().toObject(), indent + 1);
        } else if (it.value().isArray()) {
            std::cout << qPrintable(prefix) << qPrintable(it.key()) << ": [" << it.value().toArray().size()
                      << " items]\n";
        } else {
            std::cout << qPrintable(prefix) << qPrintable(it.key()) << ": "
                      << qPrintable(it.value().toVariant().toString()) << "\n";
        }
    }
}

static QString readLine(const QString &prompt)
{
    std::cout << qPrintable(prompt) << std::flush;
    std::string line;
    std::getline(std::cin, line);
    return QString::fromStdString(line).trimmed();
}

static int run(NeteaseClient *client, const QString &phone, int ctcode)
{
    QCoro::waitFor([&]() -> QCoro::Task<void> {
        // Step 1: Send captcha
        std::cout << "\n━━━ Step 1: Sending captcha to +" << ctcode << " " << qPrintable(phone) << " ━━━\n";

        auto sendResult = co_await client->sendCaptcha(phone, ctcode);
        if (sendResult.isError()) {
            std::cerr << "❌ sendCaptcha failed (code=" << sendResult.error().code()
                      << "): " << qPrintable(sendResult.error().message()) << "\n";
            co_return;
        }
        std::cout << "✅ Captcha sent successfully!\n";

        // Step 2: Read captcha from user
        std::cout << "\n━━━ Step 2: Enter the captcha you received ━━━\n";
        QString captcha = readLine("Captcha: ");
        if (captcha.isEmpty()) {
            std::cerr << "❌ Empty captcha, aborting.\n";
            co_return;
        }

        // Step 3: Verify captcha (optional, but validates before login)
        std::cout << "\n━━━ Step 3: Verifying captcha ━━━\n";
        auto verifyResult = co_await client->verifyCaptcha(phone, captcha, ctcode);
        if (verifyResult.isError()) {
            std::cerr << "⚠️  verifyCaptcha returned (code=" << verifyResult.error().code()
                      << "): " << qPrintable(verifyResult.error().message()) << "\n";
            std::cerr << "    Proceeding to login anyway (some APIs skip verify)...\n";
        } else {
            std::cout << "✅ Captcha verified!\n";
        }

        // Step 4: Login with captcha
        std::cout << "\n━━━ Step 4: Logging in ━━━\n";
        auto loginResult = co_await client->loginByCaptcha(phone, captcha, ctcode);
        if (loginResult.isError()) {
            std::cerr << "❌ loginByCaptcha failed (code=" << loginResult.error().code()
                      << "): " << qPrintable(loginResult.error().message()) << "\n";
            co_return;
        }

        const auto &login = loginResult.data();
        std::cout << "✅ Login successful!\n\n";
        std::cout << "  User ID:  " << qPrintable(login.userId) << "\n";
        std::cout << "  Nickname: " << qPrintable(login.nickname) << "\n";
        if (!login.avatarUrl.isEmpty()) {
            std::cout << "  Avatar:   " << qPrintable(login.avatarUrl.toString()) << "\n";
        }
        if (!login.cookie.isEmpty()) {
            std::cout << "  Cookie:   " << qPrintable(login.cookie.left(60)) << "...\n";
        }

        // Step 5: Verify session
        std::cout << "\n━━━ Step 5: Verifying session ━━━\n";
        auto accountResult = co_await client->getCurrentUserAccount();
        if (accountResult.isSuccess()) {
            QJsonObject profile = accountResult.data()[QLatin1String("profile")].toObject();
            std::cout << "✅ Session valid!\n";
            printJson(profile);
        } else {
            std::cerr << "⚠️  getCurrentUserAccount: " << qPrintable(accountResult.error().message()) << "\n";
        }
    }());

    return 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Parse args
    QString phone;
    int ctcode = 86;

    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromUtf8(argv[i]);
        if (arg.startsWith(QStringLiteral("--phone="))) {
            phone = arg.mid(8);
        } else if (arg.startsWith(QStringLiteral("--ctcode="))) {
            ctcode = arg.mid(9).toInt();
        } else if (arg == QStringLiteral("--help") || arg == QStringLiteral("-h")) {
            std::cout << "Usage: NeteaseCaptchaLogin [--phone=PHONE] [--ctcode=86]\n\n"
                      << "Interactive captcha-based login for NetEase Cloud Music.\n\n"
                      << "Options:\n"
                      << "  --phone=PHONE    Phone number (prompted if omitted)\n"
                      << "  --ctcode=CODE    Country code, default 86 (China)\n";
            return 0;
        }
    }

    // Prompt for phone if not provided
    if (phone.isEmpty()) {
        phone = readLine(QStringLiteral("Phone number: "));
        if (phone.isEmpty()) {
            std::cerr << "No phone number provided.\n";
            return 1;
        }
    }

    std::cout << "Netease Captcha Login\n"
              << "━━━━━━━━━━━━━━━━━━━━━\n"
              << "Phone: +" << ctcode << " " << qPrintable(phone) << "\n";

    // Setup client
    HttpClient http;
    NeteaseClient client(&http);

    // Visit homepage to get __csrf
    std::cout << "Initializing session...\n";
    QCoro::waitFor(client.ensureWeapiSession());

    return run(&client, phone, ctcode);
}
