/*

Copyright (c) 2003, Arvid Norberg
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#include <algorithm>
#include "libtorrent/entry.hpp"
#include <boost/bind.hpp>
#include <boost/next_prior.hpp>

#if defined(_MSC_VER)
namespace std
{
	using ::isprint;
}
#define for if (false) {} else for
#endif

namespace
{
	template <class T>
	void call_destructor(T* o)
	{
		assert(o);
		o->~T();
	}

	struct compare_string
	{
		compare_string(char const* s): m_str(s)  {}
	
		bool operator()(
			std::pair<std::string
			, libtorrent::entry> const& e) const
		{
			return e.first == m_str;
		}
		char const* m_str;
	};
	
	// bubble_sort code does not work with vc++2005, and is rather useless with a std::list anyway.
	bool sortpred(const std::pair<std::string,libtorrent::entry> &left, const std::pair<std::string,libtorrent::entry> &right) {
		return left.first < right.first;
	}

	/*template <class It, class Pred>
	void bubble_sort(It start, It end, Pred p)
	{
		if (start == end) return;
		--end;
		for (It i = start; i != end; ++i)
		{
			bool unsorted = false;
			for (It j = i; j != end; ++j)
			{
				It next = boost::next(j);
				if (!p(*j, *next))
				{
					swap(*j, *next);
					unsorted = true;
				}
			}
			if (!unsorted) return;
		}
	}*/
}

namespace libtorrent
{
	namespace detail
	{
		char const*	integer_to_str(char* buf, int size, entry::integer_type val)
		{
			int sign = 0;
			if (val < 0)
			{
				sign = 1;
				val = -val;
			}
			buf[--size] = '\0';
			if (val == 0) buf[--size] = '0';
			for (; size > sign && val != 0;)
			{
				buf[--size] = '0' + char(val % 10);
				val /= 10;
			}
			if (sign) buf[--size] = '-';
			return buf + size;
		}
	}

	entry& entry::operator[](char const* key)
	{
		dictionary_type::iterator i = std::find_if(
			dict().begin()
			, dict().end()
			, compare_string(key));
		if (i != dict().end()) return i->second;
		dictionary_type::iterator ret = dict().insert(
			dict().end()
			, std::make_pair(std::string(key), entry()));
		return ret->second;
	}


	entry& entry::operator[](std::string const& key)
	{
		return (*this)[key.c_str()];
	}

	entry* entry::find_key(char const* key)
	{
		dictionary_type::iterator i = std::find_if(
			dict().begin()
			, dict().end()
			, compare_string(key));
		if (i == dict().end()) return 0;
		return &i->second;
	
	}

	entry const* entry::find_key(char const* key) const
	{
		dictionary_type::const_iterator i = std::find_if(
			dict().begin()
			, dict().end()
			, compare_string(key));
		if (i == dict().end()) return 0;
		return &i->second;
	
	}
	
	
	const entry& entry::operator[](char const* key) const
	{
		dictionary_type::const_iterator i = std::find_if(
			dict().begin()
			, dict().end()
			, compare_string(key));
		if (i == dict().end()) throw type_error("key not found");
		return i->second;
	}

	const entry& entry::operator[](std::string const& key) const
	{
		return (*this)[key.c_str()];
	}

	entry::entry(const dictionary_type& v)
	{
		new(data) dictionary_type(v);
		m_type = dictionary_t;
	}

	entry::entry(const string_type& v)
	{
		new(data) string_type(v);
		m_type = string_t;
	}

	entry::entry(const list_type& v)
	{
		new(data) list_type(v);
		m_type = list_t;
	}

	entry::entry(const integer_type& v)
	{
		new(data) integer_type(v);
		m_type = int_t;
	}

	void entry::operator=(const dictionary_type& v)
	{
		destruct();
		new(data) dictionary_type(v);
		m_type = dictionary_t;
	}

	void entry::operator=(const string_type& v)
	{
		destruct();
		new(data) string_type(v);
		m_type = string_t;
	}

	void entry::operator=(const list_type& v)
	{
		destruct();
		new(data) list_type(v);
		m_type = list_t;
	}

	void entry::operator=(const integer_type& v)
	{
		destruct();
		new(data) integer_type(v);
		m_type = int_t;
	}

	bool entry::operator==(entry const& e) const
	{
		if (m_type != e.m_type) return false;

		switch(m_type)
		{
		case int_t:
			return integer() == e.integer();
		case string_t:
			return string() == e.string();
		case list_t:
			return list() == e.list();
		case dictionary_t:
			return dict() == e.dict();
		default:
			assert(m_type == undefined_t);
			return true;
		}
	}

	void entry::construct(data_type t)
	{
		m_type = t;
		switch(m_type)
		{
		case int_t:
			new(data) integer_type;
			break;
		case string_t:
			new(data) string_type;
			break;
		case list_t:
			new(data) list_type;
			break;
		case dictionary_t:
			new (data) dictionary_type;
			break;
		default:
			assert(m_type == undefined_t);
			m_type = undefined_t;
		}
	}

	void entry::copy(const entry& e)
	{
		m_type = e.m_type;
		switch(m_type)
		{
		case int_t:
			new(data) integer_type(e.integer());
			break;
		case string_t:
			new(data) string_type(e.string());
			break;
		case list_t:
			new(data) list_type(e.list());
			break;
		case dictionary_t:
			new (data) dictionary_type(e.dict());
			break;
		default:
			m_type = undefined_t;
		}
	}

	void entry::destruct()
	{
		switch(m_type)
		{
		case int_t:
			call_destructor(reinterpret_cast<integer_type*>(data));
			break;
		case string_t:
			call_destructor(reinterpret_cast<string_type*>(data));
			break;
		case list_t:
			call_destructor(reinterpret_cast<list_type*>(data));
			break;
		case dictionary_t:
			call_destructor(reinterpret_cast<dictionary_type*>(data));
			break;
		default:
			assert(m_type == undefined_t);
			break;
		}
	}

	void entry::sort()
	{
		dict().sort(sortpred);
	}

	void entry::print(std::ostream& os, int indent) const
	{
		assert(indent >= 0);
		for (int i = 0; i < indent; ++i) os << " ";
		switch (m_type)
		{
		case int_t:
			os << integer() << "\n";
			break;
		case string_t:
			{
				bool binary_string = false;
				for (std::string::const_iterator i = string().begin(); i != string().end(); ++i)
				{
					if (!std::isprint(static_cast<unsigned char>(*i)))
					{
						binary_string = true;
						break;
					}
				}
				if (binary_string)
				{
					os.unsetf(std::ios_base::dec);
					os.setf(std::ios_base::hex);
					for (std::string::const_iterator i = string().begin(); i != string().end(); ++i)
						os << static_cast<unsigned int>((unsigned char)*i);
					os.unsetf(std::ios_base::hex);
					os.setf(std::ios_base::dec);
					os << "\n";
				}
				else
				{
					os << string() << "\n";
				}
			} break;
		case list_t:
			{
				os << "list\n";
				for (list_type::const_iterator i = list().begin(); i != list().end(); ++i)
				{
					i->print(os, indent+1);
				}
			} break;
		case dictionary_t:
			{
				os << "dictionary\n";
				for (dictionary_type::const_iterator i = dict().begin(); i != dict().end(); ++i)
				{
					for (int j = 0; j < indent+1; ++j) os << " ";
					os << "[" << i->first << "]";
					if (i->second.type() != entry::string_t && i->second.type() != entry::int_t) os << "\n";
					else os << " ";
					i->second.print(os, indent+2);
				}
			} break;
		default:
			os << "<uninitialized>\n";
		}
	}
}
