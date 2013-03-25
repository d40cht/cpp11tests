#pragma once

#include <vector>

template<typename T>
void mergeSort( std::vector<T>& data )
{
    int length = data.size();
    std::vector<T> buf( data.begin(), data.end() );
    
    auto iteration = [length]( int span, const std::vector<T>& from, std::vector<T>& to ) -> void
    {
        auto interleave = [length, span, &from, &to]( int s ) -> void
        {
            auto s1 = s;
            auto e1 = s + span;
            auto s2 = e1;
            auto e2 = std::min( s1 + span*2, length );
            auto out = s;
            
            while ( out < e2 )
            {
                // Left series exhausted
                if ( s1 >= e1 )
                {
                    to[out] = from[s2];
                    s2 += 1;
                }
                // Right series exhausted
                else if ( s2 >= e2 )
                {
                    to[out] = from[s1];
                    s1 += 1;
                }
                else
                {
                    auto l = from[s1];
                    auto r = from[s2];
                    if ( l < r )
                    {
                        to[out] = l;
                        s1 += 1;
                    }
                    else
                    {
                        to[out] = r;
                        s2 += 1;
                    }
                }
                out += 1;
            }
        };
        for ( int i = 0; i < length; i += (span*2) )
        {
            interleave( i );
        }
    };
    
    auto a = &data;
    auto b = &buf;
    for ( int span = 1; span < length; span *= 2 )
    {
        iteration( span, *a, *b );
        auto tmp = b;
        b = a;
        a = tmp;
    }

    if ( a != &data )
    {
        data = buf;
    //std::swap( data, buf );
    }
}

