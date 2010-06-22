/*
Copyright 2010  Christian Vetter veaac.fdirct@gmail.com

This file is part of MoNav.

MoNav is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MoNav is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MoNav.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TRIE_H
#define TRIE_H

#include <QString>
#include <vector>
#include "utils/coordinates.h"
#include "utils/utils.h"

namespace utt
{

struct CityData {
	UnsignedCoordinate coordinate;

	size_t GetSize() const {
		return 2 * sizeof( unsigned );
	}

	void Write( char* buffer ) const {
		*( ( unsigned* ) buffer ) = coordinate.x;
		buffer += sizeof( unsigned );
		*( ( unsigned* ) buffer ) = coordinate.y;
	}

	void Read( const char* buffer ) {
		coordinate.x = readUnaligned< unsigned >( buffer );
		buffer += sizeof( unsigned );
		coordinate.y = readUnaligned< unsigned >( buffer );
	}
};

struct Data {
	unsigned start;
	unsigned end;

	size_t GetSize() const {
		return 2 * sizeof( unsigned );
	}

	void Write( char* buffer ) const {
		*( ( unsigned* ) buffer ) = start;
		buffer += sizeof( unsigned );
		*( ( unsigned* ) buffer ) = end;
	}

	void Read( const char* buffer ) {
		start = readUnaligned< unsigned >( buffer );
		buffer += sizeof( unsigned );
		end = readUnaligned< unsigned >( buffer );
	}

	bool operator==( const Data& right ) const {
		return start == right.start && end == right.end;
	}
};

struct Label {
	QString string;
	unsigned index;
	unsigned importance;

	bool operator<( const Label& right ) const {
		return string < right.string;
	}

	bool operator==( const Label& right ) const {
		return string == right.string && importance == right.importance;
	}

	size_t GetSize() const {
		size_t result = 0;
		result += sizeof( unsigned );
		result += sizeof( unsigned );
		result += strlen( string.toUtf8().constData() ) + 1;
		return result;
	}

	void Write( char* buffer ) const {
		*( ( unsigned* ) buffer ) = index;
		buffer += sizeof( unsigned );
		*( ( unsigned* ) buffer ) = importance;
		buffer += sizeof( unsigned );
		strcpy( buffer, string.toUtf8().constData() );
	}

	void Read( const char* buffer ) {
		index = readUnaligned< unsigned >( buffer );
		buffer += sizeof( unsigned );
		importance = readUnaligned< unsigned >( buffer );
		buffer += sizeof( unsigned );
		string = QString::fromUtf8( buffer );
	}
};

struct Node {
	QVector< Data > dataList;
	QVector< Label > labelList;

	size_t GetSize() const {
		size_t result = 0;
		result += sizeof( unsigned short );
		result += sizeof( unsigned char );
		foreach ( const Label& label, labelList )
			result += label.GetSize();
		foreach ( const Data& data, dataList )
			result += data.GetSize();
		return result;
	}

	void Write( char* buffer ) const {
		assert( labelList.size() <= std::numeric_limits< unsigned short >::max() );
		assert( dataList.size() <= std::numeric_limits< unsigned char >::max() );
		*( ( unsigned short* ) buffer ) = labelList.size();
		buffer += sizeof( unsigned short );
		*( ( unsigned char* ) buffer ) = dataList.size();
		buffer += sizeof( unsigned char );
		foreach ( const Label& label, labelList ) {
			label.Write( buffer );
			buffer += label.GetSize();
		}
		foreach ( const Data& data, dataList ) {
			data.Write( buffer );
			buffer += data.GetSize();
		}
	}

	void Read( const char* buffer ) {
		labelList.resize( readUnaligned< unsigned short >( buffer ) );
		buffer += sizeof( unsigned short );
		dataList.resize( readUnaligned< unsigned char >( buffer ) );
		buffer += sizeof( unsigned char );
		for( int i = 0; i < ( int ) labelList.size(); i++ ) {
			labelList[i].Read( buffer );
			buffer += labelList[i].GetSize();
		}
		for( int i = 0; i < ( int ) dataList.size(); i++ ) {
			dataList[i].Read( buffer );
			buffer += dataList[i].GetSize();
		}
	}

	bool operator== ( const Node& right ) const {
		if ( dataList.size() != right.dataList.size() )
			return false;
		if ( labelList.size() != right.labelList.size() )
			return false;
		for ( int i = 0; i < ( int ) dataList.size(); ++i ) {
			if ( !( dataList[i] == right.dataList[i] ) )
				return false;
		}
		for ( int i = 0; i < ( int ) labelList.size(); ++i ) {
			if ( !( labelList[i] == right.labelList[i] ) )
				return false;
		}
		return true;
	}
};

}

#endif // TRIE_H