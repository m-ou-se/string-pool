#if (__cplusplus <= 201402L || !__has_include(<optional>)) && __has_include(<experimental/optional>)
#include <experimental/optional>
namespace string_pool {
using std::experimental::optional;
using std::experimental::nullopt;
}
#else
#include <optional>
namespace string_pool {
using std::optional;
using std::nullopt;
}
#endif
