/*
 *	srecord - manipulate eprom load files
 *	Copyright (C) 1998 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: interface definition for lib/srec/output/file.cc
 */

#ifndef INCLUDE_SREC_OUTPUT_FILE_H
#define INCLUDE_SREC_OUTPUT_FILE_H

#pragma interface

#include <string>
#include <srec/output.h>

class srec_output_file: public srec_output
{
public:
	srec_output_file();
	srec_output_file(const char *);
	virtual ~srec_output_file();
	virtual const string filename() const;

protected:
	void put_char(int);
	void put_nibble(int);
	void put_byte(int);
	void checksum_reset();
	int checksum_get();

private:
	string file_name;
	int line_number;
	void *fp;
	int checksum;

	srec_output_file(const srec_output_file &);
	srec_output_file &operator=(const srec_output_file &);
};

#endif /* INCLUDE_SREC_OUTPUT_FILE_H */