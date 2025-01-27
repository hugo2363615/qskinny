/******************************************************************************
 * QSkinny - Copyright (C) 2016 Uwe Rathmann
 * This file may be used under the terms of the QSkinny License, Version 1.0
 *****************************************************************************/

#ifndef QSK_BOX_RENDERER_COLOR_MAP_H
#define QSK_BOX_RENDERER_COLOR_MAP_H

#include <QskGradient.h>
#include <QskVertex.h>

#include <cassert>

class QskBoxShapeMetrics;

namespace QskVertex
{
    class ColorMapNone
    {
      public:
        constexpr inline Color colorAt( qreal ) const
        {
            return Color();
        }
    };

    class ColorMapSolid
    {
      public:
        constexpr inline ColorMapSolid() = default;

        inline ColorMapSolid( const QskGradient& gradient )
            : m_color( gradient.rgbStart() )
        {
        }

        inline Color colorAt( qreal ) const
        {
            return m_color;
        }

      private:
        const Color m_color;
    };

    class ColorMapGradient
    {
      public:
        inline ColorMapGradient( const QskGradient& gradient )
            : m_color1( gradient.rgbStart() )
            , m_color2( gradient.rgbEnd() )
        {
        }

        inline Color colorAt( qreal value ) const
        {
            return m_color1.interpolatedTo( m_color2, value );
        }

      private:
        const Color m_color1;
        const Color m_color2;
    };

    class ColorIterator
    {
      public:
        static inline bool advance()
        {
            return false;
        }

        inline qreal value() const
        {
            assert( false );
            return 0.0;
        }

        inline Color color() const
        {
            assert( false );
            return Color();
        }

        static inline bool isDone()
        {
            return true;
        }
    };

    class SolidColorIterator : public ColorIterator
    {
      public:
        inline SolidColorIterator( const QColor& color )
            : m_color( color )
        {
        }

        inline Color colorAt( qreal ) const
        {
            return m_color;
        }

      private:
        const Color m_color;
    };

    class TwoColorIterator01 : public ColorIterator
    {
      public:
        inline TwoColorIterator01( const QColor& color1, const QColor& color2 )
            : m_color1( color1 )
            , m_color2( color2 )
        {
        }

        inline Color colorAt( qreal value ) const
        {
            return m_color1.interpolatedTo( m_color2, value );
        }

      private:
        const Color m_color1, m_color2;
    };

    class TwoColorIterator : public ColorIterator
    {
      public:
        inline TwoColorIterator( qreal value1, qreal value2,
                const QColor& color1, const QColor& color2 )
            : m_value1( value1 )
            , m_range( value2 - value1 )
            , m_color1( color1 )
            , m_color2( color2 )
        {
        }

        inline Color colorAt( qreal value ) const
        {
            const qreal r = ( value - m_value1 ) / m_range;
            return m_color1.interpolatedTo( m_color2, r );
        }

      private:
        const qreal m_value1, m_range;
        const Color m_color1, m_color2;
    };

    class GradientColorIterator : public ColorIterator
    {
      public:
        inline GradientColorIterator( qreal value1, qreal value2,
                const QskGradientStops& stops )
            : m_value1( value1 )
            , m_value2( value2 )
            , m_stops( stops )
        {
            if ( stops.first().position() > 0.0 )
            {
                m_color1 = m_color2 = stops[ 0 ].rgb();
                m_index = 0;
            }
            else
            {
                m_color1 = stops[ 0 ].rgb();
                m_color2 = stops[ 1 ].rgb();
                m_index = 1;
            }

            m_finalIndex = m_stops.count() - 1;
            if ( stops.last().position() < 1.0 )
                m_finalIndex++;

            m_valueStep1 = value1;
            m_valueStep2 = valueAt( stops[ m_index ].position() );
            m_stepSize = m_valueStep2 - m_valueStep1;
        }

        inline qreal value() const
        {
            return m_valueStep2;
        }

        inline Color color() const
        {
            return m_color2;
        }

        inline Color colorAt( qreal value ) const
        {
            const qreal r = ( value - m_valueStep1 ) / m_stepSize;
            return m_color1.interpolatedTo( m_color2, r );
        }

        inline bool advance()
        {
            m_index++;

            m_color1 = m_color2;
            m_valueStep1 = m_valueStep2;

            if ( m_index >= m_stops.count() )
            {
                m_color2 = m_color1;
                m_valueStep2 = valueAt( 1.0 );
            }
            else
            {
                const auto& stop = m_stops[ m_index ];

                m_color2 = stop.rgb();
                m_valueStep2 = valueAt( stop.position() );
            }

            m_stepSize = m_valueStep2 - m_valueStep1;

            return !isDone();
        }

        inline bool isDone() const
        {
            return m_index >= m_finalIndex;
        }

      private:
        inline qreal valueAt( qreal pos ) const
        {
            return m_value1 + pos * ( ( m_value2 - m_value1 ) );
        }

        const qreal m_value1, m_value2;
        const QskGradientStops m_stops;

        int m_index, m_finalIndex;
        qreal m_valueStep1, m_valueStep2, m_stepSize;
        Color m_color1, m_color2;
    };

    template< class ContourIterator, class ColorIterator >
    ColoredLine* fillOrdered( ContourIterator& contourIt,
        ColorIterator& colorIt, ColoredLine* line )
    {
        do
        {
            while ( !colorIt.isDone() && ( colorIt.value() < contourIt.value() ) )
            {
                contourIt.setGradientLine( colorIt, line++ );
                colorIt.advance();
            }

            contourIt.setContourLine( colorIt, line++ );

        } while ( contourIt.advance() );

        return line;
    }

    template< class ContourIterator >
    ColoredLine* fillOrdered( ContourIterator& contourIt,
        qreal value1, qreal value2, const QskGradient& gradient, ColoredLine* line )
    {
        if ( gradient.stepCount() == 1 )
        {
            if ( value2 == 1.0 && value1 == 0.0 )
            {
                TwoColorIterator01 colorIt( gradient.rgbStart(), gradient.rgbEnd() );
                line = fillOrdered( contourIt, colorIt, line );
            }
            else
            {
                TwoColorIterator colorIt( value1, value2,
                    gradient.rgbStart(), gradient.rgbEnd() );

                line = fillOrdered( contourIt, colorIt, line );
            }
        }
        else
        {
            GradientColorIterator colorIt( value1, value2, gradient.stops() );
            line = fillOrdered( contourIt, colorIt, line );
        }

        return line;
    }
}

#endif
