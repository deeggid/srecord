//
//      srecord - The "srecord" program.
//      Copyright (C) 2007 Peter Miller
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

#include <lib/srec/arglex.h>
#include <lib/srec/input/generator.h>
#include <lib/srec/input/generator/constant.h>
#include <lib/srec/input/generator/random.h>
#include <lib/srec/input/generator/repeat.h>
#include <lib/srec/record.h>


srec_input_generator::~srec_input_generator()
{
}


srec_input_generator::srec_input_generator(const interval &a_range) :
    range(a_range)
{
}


int
srec_input_generator::read(srec_record &result)
{
    //
    // If there is not data left to generate,
    // signal end-of-file
    //
    if (range.empty())
        return 0;

    //
    // Calculate the address range for this chunk of data.  Use the
    // biggest record size available.
    //
    unsigned long addr = range.get_lowest();
    interval partial(addr, addr + srec_record::max_data_length);
    partial *= range;

    //
    // Only supply the first sub-interval, if the generation range has
    // holes in it.
    //
    partial.first_interval_only();

    //
    // Generate the data and build the result record.
    //
    result.set_type(srec_record::type_data);
    result.set_address(addr);
    result.set_length(1);
    interval::data_t size = partial.get_highest() - addr;
    for (interval::data_t j = 0; j < size; ++j)
    {
        result.set_data_extend(j, generate_data(addr + j));
    }

    //
    // Reduce the amount of data left to be generated.
    //
    range -= partial;

    //
    // Report that another record is available.
    //
    return 1;
}


srec_input *
srec_input_generator::create(srec_arglex *cmdln)
{
    interval range = cmdln->get_interval("--generate");
    srec_input *result = 0;
    switch (cmdln->token_cur())
    {
    case srec_arglex::token_constant:
        {
            cmdln->token_next();
            int n = cmdln->get_number("--generate --constant");
            result = new srec_input_generator_constant(range, n);
        }
        break;

    case srec_arglex::token_random:
        {
            cmdln->token_next();
            result = new srec_input_generator_random(range);
        }
        break;

    case srec_arglex::token_repeat_data:
        {
            cmdln->token_next();
            size_t length = 0;
            size_t maxlen = 16;
            unsigned char *data = new unsigned char [maxlen];
            for (;;)
            {
                //
                // Get the next byte value from the command line
                // (there must be at least one).
                //
                long byte_value = cmdln->get_number("--repeat-data");

                //
                // Make sure there is room for the next byte in the data array
                //
                if (length >= maxlen)
                {
                    size_t new_maxlen = maxlen * 2 + 16;
                    unsigned char *new_data = new unsigned char [new_maxlen];
                    memcpy(new_data, data, length);
                    delete [] data;
                    data = new_data;
                    maxlen = new_maxlen;
                }
                if (byte_value < 0 || byte_value > 255)
                {
                    cmdln->fatal_error
                    (
                        "data byte %ld out of range [0..255]",
                        byte_value
                    );
                }

                //
                // Stash this byte
                //
                data[length++] = byte_value;

                //
                // If there are no more numbers on the command line, we
                // are done.
                //
                if (!cmdln->can_get_number())
                    break;
            }

            //
            // Build our new input data source.
            //
            if (length == 1)
                result = new srec_input_generator_constant(range, data[0]);
            else
                result = new srec_input_generator_repeat(range, data, length);
            delete [] data;
        }
        break;

    case srec_arglex::token_repeat_string:
        {
            cmdln->token_next();
            string s = cmdln->get_string("--repeat-string");
            size_t len = s.size();
            switch (len)
            {
            case 0:
                cmdln->fatal_error("--repeat-string value may not be empty");
                // NOTREACHED

            case 1:
                result = new srec_input_generator_constant(range, s[0]);
                break;

            default:
                result =
                    new srec_input_generator_repeat
                    (
                        range,
                        (unsigned char *)s.c_str(),
                        s.size()
                    );
                break;
            }
        }
        break;

    default:
        cmdln->fatal_error
        (
            "the --generate option needs to be followed by a generation "
                "type (e.g. --constant)"
        );
        // NOTREACHED
    }
    return result;
}


void
srec_input_generator::disable_checksum_validation()
{
    // Do nothing.
    // None of the generators have checksums.
}