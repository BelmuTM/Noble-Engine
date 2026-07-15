/********************************************************************************/
/*                                                                              */
/*    Noble Shaders                                                             */
/*    Copyright (C) 2026  Belmu                                                 */
/*                                                                              */
/*    This program is free software: you can redistribute it and/or modify      */
/*    it under the terms of the GNU General Public License as published by      */
/*    the Free Software Foundation, either version 3 of the License, or         */
/*    (at your option) any later version.                                       */
/*                                                                              */
/*    This program is distributed in the hope that it will be useful,           */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*    GNU General Public License for more details.                              */
/*                                                                              */
/*    You should have received a copy of the GNU General Public License         */
/*    along with this program.  If not, see <https://www.gnu.org/licenses/>.    */
/*                                                                              */
/********************************************************************************/

#define PALETTE 9

#if PALETTE == 1
    static const int paletteSize = 7;

    // Smooth Polished Silver
    static const float3 palette[paletteSize] = float3[paletteSize](
        float3(0.000, 0.000, 0.000),
        float3(0.157, 0.129, 0.122),
        float3(0.290, 0.224, 0.239),
        float3(0.404, 0.345, 0.298),
        float3(0.690, 0.573, 0.655),
        float3(0.949, 0.839, 0.886),
        float3(1.000, 1.000, 1.000)
    );
#elif PALETTE == 2
    static const int paletteSize = 8;

    // Hortensia Diamond
    static const float3 palette[paletteSize] = float3[paletteSize](
        float3(0.200, 0.169, 0.282),
        float3(0.235, 0.251, 0.349),
        float3(0.298, 0.345, 0.494),
        float3(0.537, 0.467, 0.788),
        float3(0.737, 0.553, 1.000),
        float3(0.988, 0.608, 0.827),
        float3(0.855, 0.729, 1.000),
        float3(0.831, 0.980, 1.000)
    );
#elif PALETTE == 3
    static const int paletteSize = 8;

    // St 8 Greenery
    static const float3 palette[paletteSize] = float3[paletteSize](
        float3(0.016, 0.051, 0.016),
        float3(0.067, 0.149, 0.043),
        float3(0.114, 0.251, 0.063),
        float3(0.208, 0.400, 0.078),
        float3(0.408, 0.600, 0.122),
        float3(0.706, 0.800, 0.322),
        float3(0.894, 0.949, 0.522),
        float3(0.945, 0.949, 0.902)
    );
#elif PALETTE == 4
    static const int paletteSize = 8;

    // Golden Flame
    static const float3 palette[paletteSize] = float3[paletteSize](
        float3(0.102, 0.102, 0.102),
        float3(0.176, 0.110, 0.055),
        float3(0.302, 0.176, 0.024),
        float3(0.416, 0.235, 0.043),
        float3(0.651, 0.412, 0.086),
        float3(0.890, 0.655, 0.169),
        float3(1.000, 0.882, 0.478),
        float3(0.953, 0.929, 0.918)
    );
#elif PALETTE == 5
    static const int paletteSize = 7;

    // Midnight Ablaze
    static const float3 palette[paletteSize] = float3[paletteSize](
        float3(0.075, 0.008, 0.031),
        float3(0.122, 0.020, 0.063),
        float3(0.192, 0.020, 0.118),
        float3(0.275, 0.055, 0.169),
        float3(0.486, 0.094, 0.235),
        float3(0.835, 0.235, 0.416),
        float3(1.000, 0.510, 0.455)
    );
#elif PALETTE == 6
    static const int paletteSize = 8;

    // Custodian
    static const float3 palette[paletteSize] = float3[paletteSize](
        float3(0.169, 0.212, 0.204),
        float3(0.278, 0.282, 0.282),
        float3(0.431, 0.373, 0.322),
        float3(0.635, 0.522, 0.424),
        float3(0.627, 0.635, 0.580),
        float3(0.863, 0.725, 0.627),
        float3(0.953, 0.859, 0.776),
        float3(1.000, 0.996, 0.996)
    );
#elif PALETTE == 7
    static const int paletteSize = 8;

    // Tila-Alit
    static const float3 palette[paletteSize] = float3[paletteSize](
        float3(0.216, 0.102, 0.278),
        float3(0.376, 0.145, 0.278),
        float3(0.573, 0.227, 0.278),
        float3(0.675, 0.314, 0.263),
        float3(0.765, 0.427, 0.267),
        float3(0.890, 0.580, 0.329),
        float3(0.906, 0.690, 0.424),
        float3(0.969, 0.925, 0.682)
    );
#elif PALETTE == 8
    static const int paletteSize = 8;

    // Gothic Bit
    static const float3 palette[paletteSize] = float3[paletteSize](
        float3(0.055, 0.055, 0.071),
        float3(0.102, 0.102, 0.141),
        float3(0.200, 0.200, 0.275),
        float3(0.325, 0.325, 0.451),
        float3(0.502, 0.502, 0.643),
        float3(0.651, 0.651, 0.749),
        float3(0.757, 0.757, 0.824),
        float3(0.902, 0.902, 0.925)
    );
#elif PALETTE == 9
    static const int paletteSize = 8;

    // Halloween Candy
    static const float3 palette[paletteSize] = float3[paletteSize](
        float3(0.000, 0.067, 0.133),
        float3(0.133, 0.067, 0.333),
        float3(0.467, 0.067, 0.533),
        float3(0.800 ,0.133, 0.133),
        float3(0.800, 0.533, 0.067),
        float3(0.267, 0.800, 0.267),
        float3(0.867, 0.933, 0.333),
        float3(0.933, 0.867, 1.000)
    );
#elif PALETTE == 10
    static const int paletteSize = 8;

    // Berry Nebula
    static const float3 palette[paletteSize] = float3[paletteSize](
        float3(0.051, 0.000, 0.102),
        float3(0.180, 0.039, 0.188),
        float3(0.310, 0.078, 0.275),
        float3(0.435, 0.114, 0.361),
        float3(0.431, 0.318, 0.506),
        float3(0.427, 0.522, 0.647),
        float3(0.424, 0.725, 0.788),
        float3(0.424, 0.929, 0.929)
    );
#endif

void applyColorPalette(inout float3 color, float rng) {
    float index  = luminance(color) * (paletteSize - 1);
    float dither = float(fract(index) > rng);

    index = floor(index);

    float3 c1 = palette[min(int(index)    , paletteSize - 1)];
    float3 c2 = palette[min(int(index) + 1, paletteSize - 1)];

    color = lerp(c1, c2, dither);
}
