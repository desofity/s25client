// Copyright (c) 2017 - 2017 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#include "rttrDefines.h" // IWYU pragma: keep
#include "randomMaps/elevation/DiamondSquare.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(DiamondSquareTests)

BOOST_AUTO_TEST_CASE(CreatesExpectedNumberOfHeightPoints)
{
    const int width = 16;
    const int height = 33;
    
    MapExtent size(width, height);
    RandomUtility rnd(0);
    HeightSettings settings(0,32);
    DiamondSquare ds(rnd, settings);
    
    auto result = ds.Create(size);
    
    BOOST_REQUIRE(result.size() == width * height);
}

BOOST_AUTO_TEST_SUITE_END()
