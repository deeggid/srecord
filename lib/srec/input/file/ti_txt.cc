//
//      srecord - manipulate eprom load files
//      Copyright (C) 2006, 2007 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to impliment the srec_input_file_ti_txt class
//

#include <cctype>

#include <lib/srec/input/file/ti_txt.h>
#include <lib/srec/record.h>


srec_input_file_ti_txt::~srec_input_file_ti_txt()
{
}


srec_input_file_ti_txt::srec_input_file_ti_txt(const char *filename) :
    srec_input_file(filename),
    seen_some_input(false),
    address(0),
    token(token_start_up),
    token_value(0)
{
}


void
srec_input_file_ti_txt::get_next_token()
{
    token_value = 0;
    for (;;)
    {
        int c = get_char();
        if (c < 0)
        {
            token = token_end_of_file;
            return;
        }
        switch (c)
        {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        case '\f':
            // ignore all white space
            break;

        case '\32':
            // ignore ^Z characters
            break;

        case '@':
            token = token_at;
            return;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            for (;;)
            {
                int n = get_nibble_value(c);
                // assert(n >= 0);
                token_value = (token_value << 4) | n;
                c = get_char();
                switch (c)
                {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                    break;

                default:
                    get_char_undo(c);
                    token = token_number;
                    return;
                }
            }

        case 'Q':
        case 'q':
            token = token_q;
            return;

        default:
            token = token_junk;
            return;
        }

    }
}


int
srec_input_file_ti_txt::read(srec_record &record)
{
    for (;;)
    {
        switch (token)
        {
        case token_start_up:
            get_next_token();
            if (token != token_at)
                fatal_error("data must start with an address record");
            break;

        case token_at:
            get_next_token();
            if (token != token_number)
                fatal_error("@ must be followed by an address");
            address = token_value;
            if (address & 1)
                warning("addresses should be even");
            if (address >= (1 << 20))
                warning("addresses (0x%08lX) too large", address);
            get_next_token();
            break;

        case token_number:
            {
                seen_some_input = true;
                unsigned char buffer[srec_record::max_data_length];
                size_t n = 0;
                for (;;)
                {
                    if (token_value >= 256)
                        fatal_error("byte value (%ld) too large", token_value);
                    buffer[n++] = token_value;
                    get_next_token();
                    if (token != token_number)
                        break;
                }
                record =
                    srec_record(srec_record::type_data, address, buffer, n);
                return 1;
            }

        case token_q:
        case token_end_of_file:
            if (!seen_some_input)
                fatal_error("file contains no data");
            return 0;

        case token_junk:
            fatal_error("illegal character");
        }
    }
}


const char *
srec_input_file_ti_txt::get_file_format_name()
    const
{
    return "ti-txt (MSP430)";
}