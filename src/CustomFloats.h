#pragma once
#include <cstdint>

template <uint32_t EXP_SIZE, uint32_t FRAC_SIZE>
class CustomFloat
{
	static_assert(EXP_SIZE >= 2);
	static_assert(FRAC_SIZE <= 23);

	static inline const float frac_div = 1.0f / (1 << FRAC_SIZE);
	static inline const int32_t exp_bias = (1 << (EXP_SIZE - 1)) - 1;
	static inline const uint32_t exp_border = (1 << EXP_SIZE) - 1;
	static inline const uint32_t frac_mask = (1 << FRAC_SIZE) - 1;
	static inline const uint32_t exp_mask = (1 << EXP_SIZE) - 1;

public:
	static inline const float FLOAT_MAX = ((float)frac_mask * frac_div + 1.0f) *
	                                      (float)pow(2.0f, (exp_border - 1) - exp_bias);
	static inline const float FLOAT_MIN =
		1.0f / (float)pow(2.0, FRAC_SIZE + exp_bias - 1);
	static inline const float FLOAT_EPS = 1.0f / (float)pow(2.0, FRAC_SIZE);
	static inline constexpr uint32_t SIZEOF = EXP_SIZE + FRAC_SIZE;

	static float u2f(uint32_t val)
	{
		uint32_t f = val & frac_mask;
		uint32_t e = (val >> FRAC_SIZE) & exp_mask;

		if (e == 0) {
			return f * frac_div * (float)pow(2.0f, -exp_bias + 1);
		}

		if (e < exp_border && e >= 1) {
			return ((float)f * frac_div + 1.0f) *
			       pow(2.0f, static_cast<float>(e) - exp_bias);
		}

		return std::numeric_limits<float>::max();
	}

	static uint32_t f2u(float val)
	{
		uint32_t _val = *(uint32_t*)&val;
		uint32_t f = _val & 0x7FFFFF;
		f >>= (23 - FRAC_SIZE);
		uint32_t e = (_val >> 23) & 0xFF;
		if (e > 0) {
			if (e + exp_bias > 127)
				e -= 127 - exp_bias;
			else {
				e = 0;
				f = uint32_t(round(val * pow(2.0, exp_bias - 1) * (1 << FRAC_SIZE)));
			}
		}
		if (e == exp_border) {
			e = exp_border - 1;
			f = frac_mask;
		}
		uint32_t ans = e << FRAC_SIZE;
		ans |= f & frac_mask;
		return ans;
	}
};
