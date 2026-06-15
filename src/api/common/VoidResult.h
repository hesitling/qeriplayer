/// @file VoidResult.h
/// @brief Empty result type for no-payload operations

#ifndef NERIPLAYERQT_VOIDRESULT_H
#define NERIPLAYERQT_VOIDRESULT_H

#include <QMetaType>

namespace NeriPlayerQt {

/**
 * @brief Empty struct for API operations that return no meaningful data on success
 *
 * Use `ApiResult<VoidResult>` for operations that either succeed (no data)
 * or fail (with an ApiError). This is a concrete type (not void or monostate)
 * so it works as a template parameter.
 */
struct VoidResult {};

} // namespace NeriPlayerQt

Q_DECLARE_METATYPE(NeriPlayerQt::VoidResult)

#endif // NERIPLAYERQT_VOIDRESULT_H
