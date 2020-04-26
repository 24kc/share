#include <functional>

namespace akm {

template<size_t... indexes> struct index_tuple { };

template<size_t num, size_t... indexes>
struct build_index_tuple : build_index_tuple<num-1, num-1, indexes...> { };

template<size_t... indexes>
struct build_index_tuple<0, indexes...>
{
	using type = index_tuple<indexes...>;
};

template<class Tuple>
struct invoker {
	template<class>
	struct result;

	template<class F, class... Args>
	struct result<std::tuple<F, Args...>>
	: std::invoke_result<F, Args...> { using type = std::invoke_result_t<F, Args...>; };

	template<size_t... indexes>
	typename result<Tuple>::type
	invoke(index_tuple<indexes...>)
	{ return std::invoke(std::get<indexes>(std::move(t))...); }

	typename result<Tuple>::type
	operator()()
	{
		using indices =
		typename build_index_tuple<std::tuple_size_v<Tuple>>::type;
		return invoke(indices());
	}

	Tuple t;
};

template<class... T>
using decayed_tuple = std::tuple<std::decay_t<T>...>;

template<class Callable, class... Args>
static invoker<decayed_tuple<Callable, Args...>>
make_invoker(Callable&& callable, Args&&... args)
{
	return {
		decayed_tuple<Callable, Args...>{
			std::forward<Callable>(callable),
			std::forward<Args>(args)...
		}
	};
}

} // namespace akm
