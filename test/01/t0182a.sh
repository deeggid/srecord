#!/bin/sh
#
# srecord - Manipulate EPROM load files
# Copyright (C) 2009-2011 Peter Miller
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

TEST_SUBJECT="SHA224"
. test_prelude

cat > test.in << 'fubar'
S00600004844521B
S111000048656C6C6F2C20576F726C64210A7B
S9030000FC
fubar
if test $? -ne 0; then no_result; fi

cat > test.ok << 'fubar'
S00600004844521B
S111000048656C6C6F2C20576F726C64210A7B
S11F0100C547CF5D6BF6B795ABBE4C5CC7CAC00F1D5EC17BCD74281EA89E61089C
S5030002FA
S9030000FC
fubar
if test $? -ne 0; then no_result; fi

cat > ok2 << 'fubar'
srec_cat: libgcrypt SHA224 not available
fubar
if test $? -ne 0; then no_result; fi

srec_cat test.in -sha224 0x100 -o test.out > LOG 2>&1
if test $? -ne 0
then
    # if SHA224 not available, pass by default
    if diff ok2 LOG > /dev/null 2> /dev/null
    then
        echo
        echo "    Your gcrypt library does not appear to have SHA224 support,"
        echo "    this test is therefore declaraed to pass by default."
        echo
        pass
    fi

    # some other error
    cat LOG
    fail
fi

diff test.ok test.out
if test $? -ne 0
then
    fail
fi

#
# The things tested here, worked.
# No other guarantees are made.
#
pass

# vim: set ts=8 sw=4 et :
