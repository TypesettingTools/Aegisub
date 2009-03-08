//----------------------------------------------------------------------------
// Anti-Grain Geometry (AGG) - Version 2.5
// A high quality rendering engine for C++
// Copyright (C) 2002-2006 Maxim Shemanarev
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://antigrain.com
// 
// AGG is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// AGG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AGG; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301, USA.
//----------------------------------------------------------------------------

#ifndef AGG_conv_bcspline_INCLUDED
#define AGG_conv_bcspline_INCLUDED

#include "agg_basics.h"
#include "agg_vcgen_bcspline.h"
#include "agg_conv_adaptor_vcgen.h"


namespace agg
{

    //---------------------------------------------------------conv_bcspline
    template<class VertexSource> 
    struct conv_bcspline : public conv_adaptor_vcgen<VertexSource, vcgen_bcspline>
    {
        typedef conv_adaptor_vcgen<VertexSource, vcgen_bcspline> base_type;

        conv_bcspline(VertexSource& vs) : 
            conv_adaptor_vcgen<VertexSource, vcgen_bcspline>(vs) {}

        void   interpolation_step(double v) { base_type::generator().interpolation_step(v); }
        double interpolation_step() const { return base_type::generator().interpolation_step(); }

    private:
        conv_bcspline(const conv_bcspline<VertexSource>&);
        const conv_bcspline<VertexSource>& 
            operator = (const conv_bcspline<VertexSource>&);
    };

}


#endif

