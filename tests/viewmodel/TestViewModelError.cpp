/// @file TestViewModelError.cpp
/// @brief Unit tests for ViewModelError

#include "api/common/ApiError.h"
#include "viewmodel/ViewModelError.h"

#include <QTest>

using namespace QeriPlayerQt;

class TestViewModelError : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void defaultConstruction();
    void parameterizedConstruction();
    void fromApiError_network();
    void fromApiError_auth();
    void fromApiError_rateLimit();
    void fromApiError_notFound();
    void fromApiError_otherApi();
    void factory_network();
    void factory_database();
    void factory_validation();
    void canRetry_network();
    void canRetry_rateLimit();
    void canRetry_api();
    void canRetry_auth();
    void canRetry_notFound();
    void canRetry_database();
    void canRetry_validation();
    void canRetry_unknown();
    void isAuthError_true();
    void isAuthError_false();
    void isNetworkError_true();
    void isNetworkError_false();
};

void TestViewModelError::defaultConstruction()
{
    ViewModelError error;
    QCOMPARE(error.type(), ViewModelError::ErrorType::Unknown);
    QVERIFY(error.message().isEmpty());
    QVERIFY(error.details().isEmpty());
}

void TestViewModelError::parameterizedConstruction()
{
    ViewModelError error(ViewModelError::ErrorType::Network, "Connection failed", "Timeout after 30s");
    QCOMPARE(error.type(), ViewModelError::ErrorType::Network);
    QCOMPARE(error.message(), QStringLiteral("Connection failed"));
    QCOMPARE(error.details(), QStringLiteral("Timeout after 30s"));
}

void TestViewModelError::fromApiError_network()
{
    ApiError apiError(-1, "Network error");
    ViewModelError error = ViewModelError::fromApiError(apiError);
    QCOMPARE(error.type(), ViewModelError::ErrorType::Network);
    QVERIFY(error.isNetworkError());
}

void TestViewModelError::fromApiError_auth()
{
    ApiError apiError(401, "Unauthorized");
    ViewModelError error = ViewModelError::fromApiError(apiError);
    QCOMPARE(error.type(), ViewModelError::ErrorType::Auth);
    QVERIFY(error.isAuthError());
}

void TestViewModelError::fromApiError_rateLimit()
{
    ApiError apiError(429, "Too many requests");
    ViewModelError error = ViewModelError::fromApiError(apiError);
    QCOMPARE(error.type(), ViewModelError::ErrorType::RateLimit);
}

void TestViewModelError::fromApiError_notFound()
{
    ApiError apiError(404, "Not found");
    ViewModelError error = ViewModelError::fromApiError(apiError);
    QCOMPARE(error.type(), ViewModelError::ErrorType::NotFound);
}

void TestViewModelError::fromApiError_otherApi()
{
    ApiError apiError(500, "Internal server error");
    ViewModelError error = ViewModelError::fromApiError(apiError);
    QCOMPARE(error.type(), ViewModelError::ErrorType::Api);
}

void TestViewModelError::factory_network()
{
    ViewModelError error = ViewModelError::network("No internet");
    QCOMPARE(error.type(), ViewModelError::ErrorType::Network);
    QCOMPARE(error.message(), QStringLiteral("No internet"));
}

void TestViewModelError::factory_database()
{
    ViewModelError error = ViewModelError::database("Constraint violation");
    QCOMPARE(error.type(), ViewModelError::ErrorType::Database);
    QCOMPARE(error.message(), QStringLiteral("Constraint violation"));
}

void TestViewModelError::factory_validation()
{
    ViewModelError error = ViewModelError::validation("Name is empty");
    QCOMPARE(error.type(), ViewModelError::ErrorType::Validation);
    QCOMPARE(error.message(), QStringLiteral("Name is empty"));
}

void TestViewModelError::canRetry_network()
{
    ViewModelError error = ViewModelError::network("test");
    QVERIFY(error.canRetry());
}

void TestViewModelError::canRetry_rateLimit()
{
    ApiError apiError(429, "Rate limited");
    ViewModelError error = ViewModelError::fromApiError(apiError);
    QVERIFY(error.canRetry());
}

void TestViewModelError::canRetry_api()
{
    ApiError apiError(500, "Server error");
    ViewModelError error = ViewModelError::fromApiError(apiError);
    QVERIFY(error.canRetry());
}

void TestViewModelError::canRetry_auth()
{
    ApiError apiError(401, "Unauthorized");
    ViewModelError error = ViewModelError::fromApiError(apiError);
    QVERIFY(!error.canRetry());
}

void TestViewModelError::canRetry_notFound()
{
    ApiError apiError(404, "Not found");
    ViewModelError error = ViewModelError::fromApiError(apiError);
    QVERIFY(!error.canRetry());
}

void TestViewModelError::canRetry_database()
{
    ViewModelError error = ViewModelError::database("test");
    QVERIFY(!error.canRetry());
}

void TestViewModelError::canRetry_validation()
{
    ViewModelError error = ViewModelError::validation("test");
    QVERIFY(!error.canRetry());
}

void TestViewModelError::canRetry_unknown()
{
    ViewModelError error;
    QVERIFY(!error.canRetry());
}

void TestViewModelError::isAuthError_true()
{
    ViewModelError error(ViewModelError::ErrorType::Auth, "test");
    QVERIFY(error.isAuthError());
}

void TestViewModelError::isAuthError_false()
{
    ViewModelError error(ViewModelError::ErrorType::Network, "test");
    QVERIFY(!error.isAuthError());
}

void TestViewModelError::isNetworkError_true()
{
    ViewModelError error(ViewModelError::ErrorType::Network, "test");
    QVERIFY(error.isNetworkError());
}

void TestViewModelError::isNetworkError_false()
{
    ViewModelError error(ViewModelError::ErrorType::Auth, "test");
    QVERIFY(!error.isNetworkError());
}

QTEST_MAIN(TestViewModelError)
#include "TestViewModelError.moc"
