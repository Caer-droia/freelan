/*
 * libiconvplus - A lightweight C++ wrapper around the iconv library.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libiconvplus.
 *
 * libiconvplus is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libiconvplus is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 *
 * If you intend to use libiconvplus in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file converter.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The iconv converter class.
 */

#include "converter.hpp"

#include <cstring>

namespace iconvplus
{
	template <class InputStreamType, class OutputStreamType>
	bool converter::do_convert(const iconv_instance& ic, InputStreamType& is, OutputStreamType& os, boost::system::error_code& ec, size_t* non_reversible_conversions) const
	{
		size_t counter = 0;

		if (!non_reversible_conversions) non_reversible_conversions = &counter;

		ic.reset();

		size_t itmp_len = 0;
		const char* inbuf = NULL;
		size_t otmp_len = get_output_buffer_size<char>();
		char* outbuf = get_output_buffer<char>();

		if (!ic.write_initial_state(&outbuf, &otmp_len, ec))
		{
			return false;
		}
		else
		{
			os.write(get_output_buffer<typename OutputStreamType::char_type>(), get_output_buffer_size<typename OutputStreamType::char_type>() - otmp_len / sizeof(typename OutputStreamType::char_type));
		}

		size_t result;

		while (is)
		{
			is.read(get_input_buffer<typename InputStreamType::char_type>() + itmp_len / sizeof(typename InputStreamType::char_type), get_input_buffer_size<typename InputStreamType::char_type>() - itmp_len / sizeof(typename InputStreamType::char_type));

			if (is.good() || is.eof())
			{
				itmp_len += is.gcount();
				inbuf = get_input_buffer<char>();

				do
				{
					otmp_len = get_output_buffer_size<char>();
					outbuf = get_output_buffer<char>();

					result = ic.convert(&inbuf, &itmp_len, &outbuf, &otmp_len, ec);

					os.write(get_output_buffer<typename OutputStreamType::char_type>(), get_output_buffer_size<typename OutputStreamType::char_type>() - otmp_len / sizeof(typename OutputStreamType::char_type));

					if (result == iconv_instance::ERROR_VALUE)
					{
						if (ec.value() == E2BIG)
						{
							// We check if the destination buffer will always be too small.
							if (otmp_len >= get_output_buffer_size<char>())
							{
								return false;
							}
						}
						else if (ec.value() == EINVAL)
						{
							// An incomplete multi-byte sequence was cut. Lets copy the bytes to the beginning of the next input buffer and try again.
							std::memmove(get_input_buffer<char>(), inbuf, itmp_len);
						}
						else
						{
							return false;
						}
					}

					non_reversible_conversions += result;
				}
				while ((result == iconv_instance::ERROR_VALUE) && (ec.value() == E2BIG));
			}
		}

		return true;
	}

	bool converter::convert(const iconv_instance& ic, std::istream& is, std::ostream& os, boost::system::error_code& ec, size_t* non_reversible_conversions) const
	{
		return do_convert(ic, is, os, ec, non_reversible_conversions);
	}

	bool converter::convert(const iconv_instance& ic, std::istream& is, std::wostream& os, boost::system::error_code& ec, size_t* non_reversible_conversions) const
	{
		return do_convert(ic, is, os, ec, non_reversible_conversions);
	}
}
