/// @file VoidResult.h
/// @brief Empty result type for no-payload operations

#ifndef QERIPLAYERQT_VOIDRESULT_H
#define QERIPLAYERQT_VOIDRESULT_H

#include <QMetaType>

namespace QeriPlayerQt {

/**
 * @brief Empty struct for API operations that return no meaningful data on success
 *
 * Use `ApiResult<VoidResult>` for operations that either succeed (no data)
 * or fail (with an ApiError). This is a concrete type (not void or monostate)
 * so it works as a template parameter.
 */
struct VoidResult { };

} // namespace QeriPlayerQt

Q_DECLARE_METATYPE(QeriPlayerQt::VoidResult)

#endif // QERIPLAYERQT_VOIDRESULT_H
