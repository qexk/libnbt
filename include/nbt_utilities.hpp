// libnbt++ --- A C++ library for parsing, generating and manipulating NBT
// Copyright (C) 2018 Alexandre Szymocha <alexandre@szymocha.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef __GNUC__
# pragma once
#endif // !__GNUC__
#ifndef NBT_UTILITIES_H_
# define NBT_UTILITIES_H_

# include <algorithm>
# include <memory>
# include <type_traits>
# include <variant>

namespace nbt {

# define NBT_GETTER__BUILDER_(NAME, TAG)                                 \
constexpr struct                                                         \
{                                                                        \
	constexpr operator std::size_t() const                           \
	{                                                                \
		return TAG;                                              \
	}                                                                \
                                                                         \
	template                                                         \
	<	typename _Variant                                        \
	,	typename _Tp = std::variant_alternative_t<TAG, _Variant> \
	>                                                                \
	inline std::conditional_t                                        \
	<	sizeof(_Tp) <= sizeof(std::size_t)                       \
		&& std::is_copy_constructible_v<_Tp>                     \
		&& std::is_copy_assignable_v<_Tp>                        \
	,	_Tp                                                      \
	,	std::add_lvalue_reference_t<_Tp>                         \
	>                                                                \
	operator()(std::unique_ptr<_Variant> const &nbt) const           \
	{                                                                \
		return std::get<TAG>(*nbt);                              \
	}                                                                \
                                                                         \
	template                                                         \
	<	typename _Variant                                        \
	,	typename _Tp = std::variant_alternative_t<TAG, _Variant> \
	>                                                                \
	inline std::conditional_t                                        \
	<	sizeof(_Tp) <= sizeof(std::size_t)                       \
		&& std::is_copy_constructible_v<_Tp>                     \
		&& std::is_copy_assignable_v<_Tp>                        \
	,	_Tp                                                      \
	,	std::add_lvalue_reference_t<_Tp>                         \
	>                                                                \
	operator()(_Variant const *nbt) const                            \
	{                                                                \
		return std::get<TAG>(*nbt);                              \
	}                                                                \
} NAME

NBT_GETTER__BUILDER_(byte,       0 );
NBT_GETTER__BUILDER_(short_,     1 );
NBT_GETTER__BUILDER_(int_,       2 );
NBT_GETTER__BUILDER_(long_,      3 );
NBT_GETTER__BUILDER_(float_,     4 );
NBT_GETTER__BUILDER_(double_,    5 );
NBT_GETTER__BUILDER_(byte_array, 6 );
NBT_GETTER__BUILDER_(string,     7 );
NBT_GETTER__BUILDER_(int_array,  10);
NBT_GETTER__BUILDER_(long_array, 11);

namespace detail {

template <typename _Default, bool, unsigned, class>
struct _Variant_alternative_or_default
{
	static inline constexpr auto is_default = true;
	using type = _Default;
};

template <typename _Default, unsigned _Alt, class _Variant>
struct _Variant_alternative_or_default
<	_Default
,	true
,	_Alt
,	_Variant
>
{
	static inline constexpr auto is_default = false;
	using type = std::variant_alternative_t<_Alt, _Variant>;
};

template <typename _Default, unsigned _Alt, class _Variant>
using variant_alternative_or_default =
	_Variant_alternative_or_default
	<	_Default
	,	_Alt < std::variant_size_v<_Variant>
	,	_Alt
	,	_Variant
	>;

template
<	unsigned _Alt
,	typename _Variant
,	typename _List
>
class list_wrapper
{
# ifdef NBT_UNIT_TEST_
public:
# endif // NBT_UNIT_TEST_
	static inline _List const defaulted = _List();
	_List const *cont;
public:
	using variant_or_default = variant_alternative_or_default
	<	_Variant
	,	_Alt, _Variant
	>;
	using value_type =
		std::add_const_t
		<	std::conditional_t
			<	_Alt == 8
			,	std::unique_ptr
				<	_Variant
				,	void (*) (_Variant *)
				>
			,	typename variant_or_default::type
			>
		>;
	using reference = value_type &;
	using const_reference = value_type const &;
	using difference_type = std::ptrdiff_t;
	using size_type = std::size_t;

	class iterator
	{
# ifdef NBT_UNIT_TEST_
	public:
# endif // NBT_UNIT_TEST_
		typename _List::const_iterator it;
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = list_wrapper<_Alt, _Variant, _List>
					::value_type;
		using pointer = value_type *;
		using reference = value_type &;
		using iterator_category = std::input_iterator_tag;

		iterator()
		:	it()
		{}

		iterator(typename _List::const_iterator const &it)
		:	it(it)
		{}

		inline iterator &
		operator++(void)
		{
			++it;
			return *this;
		}

		inline iterator
		operator++(int)
		{
			iterator ret(*this);
			this->operator++();
			return ret;
		}

		std::conditional_t
		<	_Alt == 8
		,	_Variant *
		,	pointer
		>
		operator->(void) const
		{
			auto const variant = reinterpret_cast<_Variant *>
			(	it->get()
			);
			if constexpr (_Alt == 8)
				return variant;
			if constexpr (variant_or_default::is_default)
				return variant;
			else
				return std::get_if<_Alt>(variant);
		}

		inline std::conditional_t
		<	_Alt == 8
		,	_Variant *
		,	reference
		>
		operator*(void) const
		{
			if constexpr (_Alt == 8)
				return reinterpret_cast<_Variant *>
				(	it->get()
				);
			else
				return *this->operator->();
		}

		inline bool
		operator==(iterator const &rhs) const
		{
			return this->it == rhs.it;
		}

		inline bool
		operator!=(iterator const &rhs) const
		{
			return this->it != rhs.it;
		}
	};
	using const_iterator = iterator const;

	list_wrapper()
	:	cont{&list_wrapper<_Alt, _Variant, _List>::defaulted}
	{}

	list_wrapper(_List const *cont)
	:	cont{cont}
	{}

	list_wrapper(list_wrapper const &a)
	:	cont{a.cont}
	{}

	~list_wrapper()
	{}

	list_wrapper &
	operator=(list_wrapper const &a)
	{
		this->cont = a.cont;
		return *this;
	}

	inline const_iterator
	begin(void) const
	{
		return iterator(this->cont->cbegin());
	}

	inline const_iterator
	end(void) const
	{
		return iterator(this->cont->cend());
	}

	inline const_iterator
	cbegin(void) const
	{
		return this->begin();
	}

	inline const_iterator
	cend(void) const
	{
		return this->end();
	}

	template
	<	unsigned _Alt_a
	,	typename _Variant_a
	,	typename _List_a
	>
	bool
	operator==(list_wrapper<_Alt_a, _Variant_a, _List_a> const &a) const
	{
		return
			this->cont == a.cont
			|| std::equal
			(	this->begin(), this->end()
			,	a.begin(), a.end()
			);
	}

	template
	<	unsigned _Alt_a
	,	typename _Variant_a
	,	typename _List_a
	>
	inline bool
	operator!=(list_wrapper<_Alt_a, _Variant_a, _List_a> const &a) const
	{
		return !this->operator==(a);
	}

	void
	swap(list_wrapper &a) noexcept
	{
		auto const tmp = this->cont;
		this->cont = a.cont;
		a.cont = tmp;
	}

	void
	swap(list_wrapper &&a) noexcept
	{
		this->operator=(std::forward(a));
	}

	friend void
	swap(list_wrapper &a, list_wrapper &b) noexcept
	{
		return a.swap(b);
	}

	inline size_type
	size(void) const
	{
		return this->cont->size();
	}
	
	inline size_type
	max_size(void) const
	{
		return this->cont->max_size();
	}

	inline bool
	empty(void) const
	{
		return this->cont->empty();
	}

	inline const_reference
	front(void) const
	{
		return *this->begin();
	}

	inline const_reference
	back(void) const
	{
		return *iterator(--this->cont->cend());
	}

	inline const_reference
	operator[](size_type const n) const
	{
		return *iterator(this->cont->cbegin() + n);
	}

	const_reference
	at(size_type const n) const
	{
		if (n >= this->size())
			throw std::out_of_range
			(	"nbt::detail::list_wrapper"
			);
		else
			return this->operator[](n);
	}
};

template
<	typename _Variant
,	typename _Compound
,	typename _String = std::variant_alternative_t<nbt::string, _Variant>
>
class compound_wrapper
{
# ifdef NBT_UNIT_TEST_
public:
# endif // NBT_UNIT_TEST_
	static inline _Compound const defaulted = _Compound();
	_Compound const *cont;
public:
	using key_type = std::decay_t<_String>;
	using mapped_type = std::decay_t<_Variant> const *;

	using value_type = std::pair<key_type const, mapped_type>;
	using reference = value_type &;
	using const_reference = value_type const &;
	using difference_type = std::ptrdiff_t;
	using size_type = std::size_t;

	using hasher = typename _Compound::hasher;
	using key_equal = typename _Compound::key_equal;

	class iterator
	{
# ifdef NBT_UNIT_TEST_
	public:
# endif // NBT_UNIT_TEST_
		typename _Compound::const_iterator it;
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = compound_wrapper::value_type;
		using pointer = value_type *;
		using reference = value_type &;
		using iterator_category = std::input_iterator_tag;

		iterator()
		:	it()
		{}

		iterator(typename _Compound::const_iterator const &it)
		:	it(it)
		{}

		inline iterator &
		operator++(void)
		{
			++it;
			return *this;
		}

		inline iterator
		operator++(int)
		{
			iterator ret(*this);
			this->operator++();
			return ret;
		}

		value_type
		operator*(void) const
		{
			auto const &pair = *it;
			return std::make_pair
			(	pair.first
			,	reinterpret_cast<_Variant *>
				(	pair.second.get()
				)
			);
		}

		std::unique_ptr<value_type>
		operator->(void) const
		{
			auto const &pair = *it;
			return std::unique_ptr<value_type>
			(	new value_type
				(	pair.first
				,	reinterpret_cast<_Variant *>
					(	pair.second.get()
					)
				)
			);
		}

		inline bool
		operator==(iterator const &rhs) const
		{
			return this->it == rhs.it;
		}

		inline bool
		operator!=(iterator const &rhs) const
		{
			return this->it != rhs.it;
		}
	};
	using const_iterator = iterator const;
	using local_iterator = iterator;
	using const_local_iterator = local_iterator const;

	compound_wrapper()
	:	cont{&compound_wrapper::defaulted}
	{}

	compound_wrapper(_Compound const *cont)
	:	cont{cont}
	{}

	compound_wrapper(compound_wrapper const &a)
	:	cont{a.cont}
	{}

	~compound_wrapper()
	{}

	compound_wrapper &
	operator=(compound_wrapper const &a)
	{
		this->cont = a.cont;
		return *this;
	}

	inline const_iterator
	begin(void) const
	{
		return iterator(this->cont->cbegin());
	}

	inline const_iterator
	end(void) const
	{
		return iterator(this->cont->cend());
	}

	inline const_iterator
	cbegin(void) const
	{
		return this->begin();
	}

	inline const_iterator
	cend(void) const
	{
		return this->end();
	}

	template
	<	typename _Variant_a
	,	typename _Compound_a
	,	typename _String_a
	>
	bool
	operator==
	(	compound_wrapper
		<	_Variant_a, _Compound_a, _String_a
		> const &a
	) const
	{
		return this->cont == a.cont;
	}

	template
	<	typename _Variant_a
	,	typename _Compound_a
	,	typename _String_a
	>
	bool
	operator!=
	(	compound_wrapper
		<	_Variant_a, _Compound_a, _String_a
		> const &a
	) const
	{
		return !this->operator==(a);
	}

	void
	swap(compound_wrapper &a) noexcept
	{
		auto const tmp = this->cont;
		this->cont = a.cont;
		a.cont = tmp;
	}

	void
	swap(compound_wrapper &&a) noexcept
	{
		this->operator=(std::forward(a));
	}

	friend void
	swap(compound_wrapper &a, compound_wrapper &b) noexcept
	{
		return a.swap(b);
	}

	friend void
	swap(compound_wrapper &a, compound_wrapper &&b) noexcept
	{
		a.operator=(std::forward(b));
	}

	friend void
	swap(compound_wrapper &&a, compound_wrapper &b) noexcept
	{
		b.operator=(std::forward(a));
	}

	inline size_type
	size(void) const
	{
		return this->cont->size();
	}
	
	inline size_type
	max_size(void) const
	{
		return this->cont->max_size();
	}

	inline bool
	empty(void) const
	{
		return this->cont->empty();
	}

	inline mapped_type
	operator[](key_type const &n) const
	{
		if
		(	auto const &it = this->cont->find(n)
		;	it != this->cont->cend()
		)
			return iterator(it)->second;
		else
			return nullptr;
	}

	inline mapped_type
	at(key_type const &n) const
	{
		if
		(	auto const &it = this->cont->find(n)
		;	it != this->cont->cend()
		)
			return iterator(it)->second;
		else
			throw std::out_of_range
			(	"nbt::detail::compound_wrapper"
			);
	}
};

} // detail

constexpr struct
{
	constexpr operator std::size_t() const
	{
		return 8;
	}

	template
	<	unsigned _Alt
	,	typename _Variant
	,	typename _List = std::variant_alternative_t<8, _Variant>
	>
	auto
	inline as(std::unique_ptr<_Variant> const &nbt) const
	{
		return detail::list_wrapper<_Alt, _Variant, _List>
		(	std::get_if<8>(nbt.get())
		);
	}

	template
	<	unsigned _Alt
	,	typename _Variant
	,	typename _List = std::variant_alternative_t<8, _Variant>
	>
	auto
	inline as(_Variant const *nbt) const
	{
		return detail::list_wrapper<_Alt, _Variant, _List>
		(	std::get_if<8>(nbt)
		);
	}

	template
	<	typename _Variant
	,	typename _Tp = std::variant_alternative_t<8, _Variant>
	>
	auto
	inline operator()(std::unique_ptr<_Variant> const &nbt) const
	{
		return detail::list_wrapper<(unsigned)(-1), _Variant, _Tp>
		(	std::get_if<8>(nbt.get())
		);
	}

	template
	<	typename _Variant
	,	typename _Tp = std::variant_alternative_t<8, _Variant>
	>
	auto
	inline operator()(_Variant const *nbt) const
	{
		return detail::list_wrapper<(unsigned)(-1), _Variant, _Tp>
		(	std::get_if<8>(nbt)
		);
	}
} list;

constexpr struct
{
	constexpr operator std::size_t() const
	{
		return 9;
	}

	template
	<	typename _Variant
	,	typename _Tp = std::variant_alternative_t<9, _Variant>
	>
	auto
	inline operator()(std::unique_ptr<_Variant> const &nbt) const
	{
		return detail::compound_wrapper<_Variant, _Tp>
		(	std::get_if<9>(nbt.get())
		);
	}

	template
	<	typename _Variant
	,	typename _Tp = std::variant_alternative_t<9, _Variant>
	>
	auto
	inline operator()(_Variant const *nbt) const
	{
		return detail::compound_wrapper<_Variant, _Tp>
		(	std::get_if<9>(nbt)
		);
	}
} compound;

# undef NBT_GETTER__BUILDER_

} // nbt

#endif // !NBT_UTILITIES_H_
