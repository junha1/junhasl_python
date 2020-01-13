#pragma once

#include "YSL_SerializationConfiguration.h"
#include "YTL/YTL.h"
#include "YSL_Common.h"
#include "YSL_Types.h"

namespace ysl
{
#define YSL_PACK(...) static constexpr auto ysl_mps = make_tuple(__VA_ARGS__);\
static constexpr bool ysl_struct_trival = true;




	template <class T, typename... Args>
	static tuple<Args& ...> ysl_pack(T* t, const tuple<Args T::* ...>& mpt)
	{
		static constexpr int n = sizeof...(Args);
		static constexpr auto range = YTL::GenAtoB<0, n - 1>::ints();

		return YTL::PM_Ints(range, [&](auto... S)
			{
				return std::tie(t->*get<S.value>(mpt)...);
			});
	}
	// const
	template <class T, typename... Args>
	static tuple<const Args& ...> ysl_pack(const T* t, const tuple<Args T::* ...>& mpt)
	{
		static constexpr int n = sizeof...(Args);
		static constexpr auto range = YTL::GenAtoB<0, n - 1>::ints();

		return YTL::PM_Ints(range, [&](auto... S)
			{
				return std::tie(t->*get<S.value>(mpt)...);
			});
	}

	struct ysl_struct_base
	{

	};
	struct ysl_struct : public ysl_struct_base
	{
		virtual ysl_struct* ysl_factory() = 0;
		virtual yobject ysl_dumps() const = 0;
		virtual void ysl_loads(const yobject& p) = 0;
	};

	template <class T>
	auto ysl_struct_tuple(T* t)
	{
		using B = typename T::ysl_parent;
		if constexpr (std::is_same_v<B, void> || std::is_same_v<B, ysl_struct_base>)
			return ysl_pack(t, T::ysl_mps);
		else
			return make_tuple(ysl_struct_tuple<B>(t), ysl_pack(t, T::ysl_mps));
	}
	template <class T>
	auto ysl_struct_tuple(const T* t)
	{
		using B = typename T::ysl_parent;
		if constexpr (std::is_same_v<B, void> || std::is_same_v<B, ysl_struct_base>)
			return ysl_pack(t, T::ysl_mps);
		else
			return make_tuple(ysl_struct_tuple<B>(t), (t, T::ysl_mps));
	}

	template <class Data>
	struct ysl_struct_final : public Data
	{
		using ysl_parent = Data;
		YSL_PACK();
		virtual ysl_struct* ysl_factory() { return new ysl_struct_final(); }
		virtual yobject ysl_dumps() const
		{
			return ysl::to(ysl_struct_tuple((Data*)this));
		}
		virtual void ysl_loads(const yobject& p)
		{
			auto t = ysl_struct_tuple((Data*)this);
			ysl::from(p, t);
			ysl_loads_notify(); // could be overriden
			// FI : just cast?
		}
		virtual void ysl_loads_notify() {}

		// constructor pass
		template <typename... Args>
		ysl_struct_final(Args&& ... args) : Data(args...)
		{}
	};
}





