#pragma once

namespace mmf::dsp::filters {

enum class FilterModelType {
    SVF = 0,
    TransistorLadder = 1,
    DiodeLadder = 2,
    LPG = 3,
    Comb = 4,
    Biquad = 5,
};

enum class FilterRouting {
    A_TO_B = 0,
    B_TO_A = 1,
    PARALLEL = 2,
};

enum class SvfMode {
    LP = 0,
    HP = 1,
    BP = 2,
    NOTCH = 3,
};

enum class LadderTap {
    POLE_1 = 0,
    POLE_2 = 1,
    POLE_3 = 2,
    POLE_4 = 3,
};

enum class CombMode {
    FEEDFORWARD = 0,
    FEEDBACK = 1,
    LP_FEEDBACK = 2,
};

enum class BiquadMode {
    LP = 0,
    HP = 1,
    BP = 2,
    NOTCH = 3,
    PEAK = 4,
};

enum class LpgMode {
    LPG = 0,
    VCA = 1,
    LP = 2,
};

} // namespace mmf::dsp::filters
