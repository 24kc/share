#include <vector>
#include <cassert>

namespace akm {

template<int... nums>
struct Mean_check : std::bool_constant<(nums && ...)> { };

template<int... nums>
class Mean {
  public:
	static_assert(Mean_check<std::greater<int>()(nums, 0)...>(), "周期必须大于0");

	Mean() : data{0}, count{0} { }

	void update(int);
	auto get_means(int index) -> std::vector<double>&;

	static constexpr int N = sizeof...(nums);
	static constexpr int T[N] = { nums... };

  private:
	std::vector<double> ave[N];
	double data[N];
	int count;
};

template<int... nums>
void
Mean<nums...>::update(int a)
{
	for (int i=0; i<N; ++i) {
		if ( count % T[i] == T[i] - 1 ) {
			data[i] += a;
			ave[i].push_back(data[i] / T[i]);
			data[i] = 0;
		} else
			data[i] += a;
	}
	++count;
}

template<int... nums>
std::vector<double>&
Mean<nums...>::get_means(int index)
{
	assert(0 <= index && index < N);
	return ave[index];
}

} // namespace akm
