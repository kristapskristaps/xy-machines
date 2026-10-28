#pragma once
#include "wide.hpp"
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkoxbow {
namespace port {
typedef int int32_t;

typedef unsigned int uint32_t;

typedef short int16_t;

typedef unsigned short uint16_t;

typedef signed char int8_t;

typedef unsigned char uint8_t;

typedef Wide int64_t;

typedef Wide uint64_t;

typedef long unsigned int uintptr_t;

typedef struct {
  int32_t len, next, step, tape_pos;
  uint8_t follows_tape;
} op1_beat_t;

static const uint32_t INC_C9[12] = {
    815363807,  863847862,  915214929,  969636440,  1027294023, 1088380105,
    1153098554, 1221665362, 1294309365, 1371273005, 1452813141, 1539201906,
};

static const uint32_t SEMI_Q16[12] = {
    65536, 69433, 73562,  77936,  82570,  87480,
    92682, 98193, 104032, 110218, 116772, 123715,
};

static const uint32_t FINE_Q16[256] = {
    65536, 65551, 65566, 65580, 65595, 65610, 65625, 65640, 65654, 65669, 65684,
    65699, 65714, 65729, 65743, 65758, 65773, 65788, 65803, 65818, 65832, 65847,
    65862, 65877, 65892, 65907, 65922, 65936, 65951, 65966, 65981, 65996, 66011,
    66026, 66041, 66056, 66071, 66085, 66100, 66115, 66130, 66145, 66160, 66175,
    66190, 66205, 66220, 66235, 66250, 66265, 66280, 66294, 66309, 66324, 66339,
    66354, 66369, 66384, 66399, 66414, 66429, 66444, 66459, 66474, 66489, 66504,
    66519, 66534, 66549, 66564, 66579, 66594, 66609, 66624, 66639, 66654, 66670,
    66685, 66700, 66715, 66730, 66745, 66760, 66775, 66790, 66805, 66820, 66835,
    66850, 66865, 66880, 66896, 66911, 66926, 66941, 66956, 66971, 66986, 67001,
    67016, 67032, 67047, 67062, 67077, 67092, 67107, 67122, 67137, 67153, 67168,
    67183, 67198, 67213, 67228, 67244, 67259, 67274, 67289, 67304, 67320, 67335,
    67350, 67365, 67380, 67395, 67411, 67426, 67441, 67456, 67472, 67487, 67502,
    67517, 67532, 67548, 67563, 67578, 67593, 67609, 67624, 67639, 67655, 67670,
    67685, 67700, 67716, 67731, 67746, 67761, 67777, 67792, 67807, 67823, 67838,
    67853, 67869, 67884, 67899, 67915, 67930, 67945, 67961, 67976, 67991, 68007,
    68022, 68037, 68053, 68068, 68083, 68099, 68114, 68129, 68145, 68160, 68176,
    68191, 68206, 68222, 68237, 68252, 68268, 68283, 68299, 68314, 68330, 68345,
    68360, 68376, 68391, 68407, 68422, 68438, 68453, 68468, 68484, 68499, 68515,
    68530, 68546, 68561, 68577, 68592, 68608, 68623, 68639, 68654, 68670, 68685,
    68701, 68716, 68732, 68747, 68763, 68778, 68794, 68809, 68825, 68840, 68856,
    68871, 68887, 68902, 68918, 68933, 68949, 68965, 68980, 68996, 69011, 69027,
    69042, 69058, 69074, 69089, 69105, 69120, 69136, 69152, 69167, 69183, 69198,
    69214, 69230, 69245, 69261, 69276, 69292, 69308, 69323, 69339, 69355, 69370,
    69386, 69402, 69417,
};

static const uint16_t SVF_G[137] = {
    5,     5,     5,     6,     6,     6,     7,     7,     8,     8,     9,
    9,     10,    10,    11,    11,    12,    13,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    23,    24,    25,    27,    29,    30,
    32,    34,    36,    38,    40,    43,    45,    48,    51,    54,    57,
    61,    64,    68,    72,    76,    81,    86,    91,    96,    102,   108,
    114,   121,   128,   136,   144,   153,   162,   171,   182,   193,   204,
    216,   229,   243,   257,   272,   289,   306,   324,   344,   364,   386,
    409,   433,   459,   487,   516,   547,   580,   615,   652,   692,   734,
    779,   826,   877,   931,   988,   1049,  1115,  1184,  1259,  1339,  1425,
    1517,  1616,  1722,  1838,  1962,  2098,  2246,  2407,  2585,  2781,  2998,
    3241,  3515,  3827,  4185,  4601,  5094,  5688,  6422,  7355,  8590,  10315,
    12918, 17344, 25861, 25861, 25861, 25861, 25861, 25861, 25861, 25861, 25861,
    25861, 25861, 25861, 25861, 25861,
};

static const int16_t TANH_Q15[257] = {
    0,     255,   511,   767,   1023,  1279,  1534,  1790,  2045,  2300,  2554,
    2809,  3062,  3316,  3569,  3822,  4074,  4326,  4577,  4828,  5078,  5328,
    5577,  5825,  6072,  6319,  6565,  6811,  7055,  7299,  7542,  7784,  8025,
    8265,  8504,  8742,  8980,  9216,  9451,  9685,  9918,  10150, 10381, 10611,
    10840, 11067, 11293, 11518, 11742, 11964, 12186, 12405, 12624, 12841, 13057,
    13272, 13485, 13697, 13908, 14117, 14325, 14531, 14736, 14940, 15142, 15342,
    15541, 15739, 15935, 16130, 16323, 16515, 16705, 16894, 17081, 17267, 17451,
    17633, 17814, 17994, 18172, 18349, 18524, 18697, 18869, 19039, 19208, 19375,
    19541, 19705, 19868, 20029, 20188, 20346, 20503, 20658, 20811, 20963, 21114,
    21263, 21410, 21556, 21701, 21844, 21985, 22125, 22264, 22401, 22536, 22670,
    22803, 22935, 23064, 23193, 23320, 23445, 23570, 23693, 23814, 23934, 24053,
    24170, 24286, 24401, 24514, 24626, 24737, 24847, 24955, 25062, 25167, 25271,
    25375, 25476, 25577, 25676, 25775, 25872, 25967, 26062, 26156, 26248, 26339,
    26429, 26518, 26606, 26692, 26778, 26862, 26946, 27028, 27109, 27190, 27269,
    27347, 27424, 27500, 27576, 27650, 27723, 27795, 27867, 27937, 28006, 28075,
    28142, 28209, 28275, 28340, 28404, 28467, 28529, 28591, 28652, 28712, 28771,
    28829, 28886, 28943, 28999, 29054, 29108, 29162, 29215, 29267, 29318, 29369,
    29419, 29468, 29517, 29565, 29612, 29658, 29704, 29750, 29794, 29838, 29882,
    29925, 29967, 30008, 30049, 30090, 30130, 30169, 30208, 30246, 30283, 30320,
    30357, 30393, 30428, 30463, 30498, 30532, 30565, 30598, 30631, 30663, 30694,
    30726, 30756, 30786, 30816, 30846, 30874, 30903, 30931, 30959, 30986, 31013,
    31039, 31065, 31091, 31116, 31141, 31166, 31190, 31214, 31238, 31261, 31284,
    31306, 31328, 31350, 31371, 31393, 31413, 31434, 31454, 31474, 31494, 31513,
    31532, 31551, 31570, 31588,
};

static const uint32_t ENV_K_ATT[128] = {
    10986490, 10505448, 10024988, 9547864, 9076568, 8613319, 8160047, 7718404,
    7289765,  6875246,  6475717,  6091822, 5724001, 5372510, 5037443, 4718755,
    4416277,  4129740,  3858787,  3602996, 3361886, 3134937, 2921596, 2721291,
    2533435,  2357436,  2192706,  2038658, 1894718, 1760324, 1634933, 1518016,
    1409066,  1307597,  1213143,  1125262, 1043531, 967552,  896946,  831356,
    770446,   713897,   661413,   612713,  567536,  525635,  486781,  450760,
    417369,   386423,   357746,   331175,  306560,  283758,  262639,  243080,
    224968,   208196,   192668,   178292,  164983,  152663,  141259,  130703,
    120934,   111892,   103524,   95780,   88614,   81983,   75846,   70169,
    64915,    60054,    55556,    51395,   47545,   43983,   40687,   37639,
    34818,    32209,    29795,    27561,   25495,   23584,   21816,   20180,
    18667,    17268,    15973,    14775,   13667,   12642,   11694,   10817,
    10006,    9256,     8561,     7919,    7325,    6776,    6268,    5798,
    5363,     4961,     4588,     4244,    3926,    3631,    3359,    3107,
    2874,     2658,     2459,     2275,    2104,    1946,    1800,    1665,
    1540,     1425,     1318,     1219,    1128,    1043,    965,     892,
};

static const uint32_t ENV_K_DEC[128] = {
    14874750, 14537265, 14172024, 13781369, 13368054, 12935148, 12485928,
    12023778, 11552103, 11074244, 10593411, 10112632, 9634704,  9162169,
    8697296,  8242069,  7798190,  7367084,  6949913,  6547589,  6160798,
    5790015,  5435527,  5097459,  4775786,  4470363,  4180936,  3907164,
    3648636,  3404881,  3175384,  2959599,  2756954,  2566867,  2388745,
    2221999,  2066042,  1920297,  1784200,  1657203,  1538775,  1428406,
    1325605,  1229903,  1140853,  1058029,  981027,   909466,   842985,
    781244,   723921,   670715,   621344,   575541,   533059,   493665,
    457141,   423284,   391904,   362825,   335881,   310919,   287796,
    266379,   246544,   228175,   211166,   195418,   180837,   167339,
    154844,   143278,   132572,   122663,   113493,   105005,   97151,
    89883,    83157,    76933,    71174,    65845,    60914,    56352,
    52132,    48226,    44613,    41271,    38178,    35317,    32670,
    30222,    27957,    25861,    23922,    22129,    20470,    18935,
    17515,    16202,    14987,    13863,    12824,    11862,    10973,
    10150,    9388,     8684,     8033,     7431,     6873,     6358,
    5881,     5440,     5032,     4654,     4305,     3982,     3684,
    3407,     3152,     2915,     2697,     2494,     2307,     2134,
    1974,     1826,
};

static const uint32_t LFO_INC[128] = {
    77913,    81938,    86171,    90622,    95304,    100227,   105405,
    110850,   116577,   122599,   128932,   135593,   142598,   149964,
    157711,   165859,   174427,   183438,   192914,   202880,   213361,
    224383,   235974,   248165,   260985,   274467,   288646,   303558,
    319239,   335731,   353075,   371315,   390497,   410670,   431885,
    454196,   477659,   502335,   528286,   555577,   584277,   614461,
    646204,   679587,   714694,   751615,   790443,   831277,   874220,
    919382,   966877,   1016826,  1069355,  1124597,  1182694,  1243791,
    1308045,  1375618,  1446682,  1521417,  1600013,  1682669,  1769595,
    1861012,  1957151,  2058257,  2164586,  2276408,  2394006,  2517679,
    2647742,  2784523,  2928371,  3079649,  3238743,  3406055,  3582011,
    3767056,  3961661,  4166319,  4381550,  4607899,  4845941,  5096281,
    5359553,  5636426,  5927601,  6233819,  6555856,  6894529,  7250698,
    7625267,  8019185,  8433454,  8869123,  9327298,  9809143,  10315880,
    10848795, 11409240, 11998637, 12618482, 13270348, 13955890, 14676846,
    15435047, 16232416, 17070976, 17952857, 18880295, 19855644, 20881380,
    21960104, 23094555, 24287612, 25542301, 26861807, 28249478, 29708836,
    31243584, 32857616, 34555029, 36340129, 38217447, 40191747, 42268038,
    44451590, 46747943,
};

static const uint16_t LFO_DELAY_BLK[128] = {
    0,     1,     3,     8,     14,    21,    31,    42,    55,    69,    85,
    103,   123,   144,   167,   192,   219,   247,   277,   308,   342,   377,
    414,   452,   492,   534,   578,   623,   670,   719,   769,   821,   875,
    930,   988,   1047,  1107,  1170,  1234,  1300,  1367,  1436,  1507,  1580,
    1654,  1730,  1808,  1887,  1969,  2052,  2136,  2222,  2310,  2400,  2492,
    2585,  2680,  2776,  2874,  2974,  3076,  3179,  3284,  3391,  3500,  3610,
    3722,  3836,  3951,  4068,  4187,  4307,  4429,  4553,  4679,  4806,  4935,
    5066,  5198,  5333,  5468,  5606,  5745,  5886,  6029,  6173,  6319,  6467,
    6617,  6768,  6921,  7076,  7232,  7390,  7550,  7711,  7875,  8039,  8206,
    8374,  8544,  8716,  8890,  9065,  9242,  9420,  9600,  9782,  9966,  10152,
    10339, 10528, 10718, 10910, 11104, 11300, 11497, 11696, 11897, 12100, 12304,
    12510, 12717, 12927, 13138, 13351, 13565, 13781,
};

static const uint16_t RCP_Q[256] = {
    32704, 32577, 32451, 32326, 32201, 32078, 31956, 31835, 31714, 31595, 31476,
    31359, 31242, 31126, 31011, 30897, 30783, 30671, 30559, 30448, 30338, 30229,
    30120, 30012, 29905, 29799, 29694, 29589, 29485, 29382, 29279, 29177, 29076,
    28976, 28876, 28777, 28679, 28581, 28484, 28387, 28292, 28197, 28102, 28008,
    27915, 27822, 27730, 27639, 27548, 27458, 27369, 27280, 27191, 27103, 27016,
    26929, 26843, 26757, 26672, 26588, 26504, 26420, 26337, 26255, 26173, 26092,
    26011, 25930, 25850, 25771, 25692, 25614, 25536, 25458, 25381, 25305, 25228,
    25153, 25078, 25003, 24928, 24855, 24781, 24708, 24636, 24564, 24492, 24420,
    24350, 24279, 24209, 24139, 24070, 24001, 23933, 23865, 23797, 23730, 23663,
    23596, 23530, 23464, 23399, 23334, 23269, 23205, 23140, 23077, 23014, 22951,
    22888, 22826, 22764, 22702, 22641, 22580, 22519, 22459, 22399, 22339, 22280,
    22221, 22162, 22104, 22046, 21988, 21931, 21873, 21816, 21760, 21704, 21648,
    21592, 21536, 21481, 21426, 21372, 21317, 21263, 21210, 21156, 21103, 21050,
    20997, 20945, 20893, 20841, 20789, 20738, 20687, 20636, 20585, 20535, 20485,
    20435, 20385, 20336, 20286, 20237, 20189, 20140, 20092, 20044, 19996, 19949,
    19901, 19854, 19807, 19761, 19714, 19668, 19622, 19576, 19531, 19485, 19440,
    19395, 19350, 19306, 19262, 19217, 19173, 19130, 19086, 19043, 19000, 18957,
    18914, 18872, 18829, 18787, 18745, 18703, 18662, 18620, 18579, 18538, 18497,
    18456, 18416, 18375, 18335, 18295, 18255, 18216, 18176, 18137, 18098, 18059,
    18020, 17982, 17943, 17905, 17867, 17829, 17791, 17753, 17716, 17678, 17641,
    17604, 17567, 17531, 17494, 17458, 17421, 17385, 17349, 17313, 17278, 17242,
    17207, 17172, 17137, 17102, 17067, 17032, 16998, 16963, 16929, 16895, 16861,
    16827, 16794, 16760, 16727, 16693, 16660, 16627, 16594, 16561, 16529, 16496,
    16464, 16432, 16400,
};

static const uint16_t RCP_S[256] = {
    4080, 4048, 4017, 3986, 3956, 3926, 3896, 3866, 3837, 3808, 3780, 3751,
    3723, 3696, 3669, 3642, 3615, 3589, 3563, 3537, 3511, 3486, 3461, 3436,
    3412, 3388, 3364, 3340, 3316, 3293, 3270, 3248, 3225, 3203, 3181, 3159,
    3138, 3116, 3095, 3074, 3053, 3033, 3013, 2993, 2973, 2953, 2934, 2914,
    2895, 2876, 2857, 2839, 2821, 2802, 2784, 2766, 2749, 2731, 2714, 2697,
    2680, 2663, 2646, 2630, 2613, 2597, 2581, 2565, 2549, 2534, 2518, 2503,
    2488, 2472, 2458, 2443, 2428, 2414, 2399, 2385, 2371, 2357, 2343, 2329,
    2315, 2302, 2288, 2275, 2262, 2249, 2236, 2223, 2210, 2198, 2185, 2173,
    2160, 2148, 2136, 2124, 2112, 2100, 2089, 2077, 2066, 2054, 2043, 2032,
    2020, 2009, 1998, 1988, 1977, 1966, 1956, 1945, 1935, 1924, 1914, 1904,
    1894, 1884, 1874, 1864, 1854, 1844, 1835, 1825, 1816, 1806, 1797, 1788,
    1779, 1769, 1760, 1751, 1742, 1734, 1725, 1716, 1707, 1699, 1690, 1682,
    1674, 1665, 1657, 1649, 1641, 1633, 1624, 1617, 1609, 1601, 1593, 1585,
    1578, 1570, 1562, 1555, 1547, 1540, 1533, 1525, 1518, 1511, 1504, 1497,
    1490, 1483, 1476, 1469, 1462, 1455, 1448, 1442, 1435, 1428, 1422, 1415,
    1409, 1402, 1396, 1390, 1383, 1377, 1371, 1365, 1359, 1353, 1346, 1340,
    1334, 1329, 1323, 1317, 1311, 1305, 1299, 1294, 1288, 1283, 1277, 1271,
    1266, 1260, 1255, 1250, 1244, 1239, 1233, 1228, 1223, 1218, 1213, 1207,
    1202, 1197, 1192, 1187, 1182, 1177, 1172, 1168, 1163, 1158, 1153, 1148,
    1144, 1139, 1134, 1130, 1125, 1120, 1116, 1111, 1107, 1102, 1098, 1093,
    1089, 1085, 1080, 1076, 1072, 1067, 1063, 1059, 1055, 1051, 1046, 1042,
    1038, 1034, 1030, 1026,
};

static const uint8_t OCT_SEMI[97] = {
    (0) << 4,      (0) << 4 | 1,  (0) << 4 | 2,  (0) << 4 | 3,  (0) << 4 | 4,
    (0) << 4 | 5,  (0) << 4 | 6,  (0) << 4 | 7,  (0) << 4 | 8,  (0) << 4 | 9,
    (0) << 4 | 10, (0) << 4 | 11, (1) << 4,      (1) << 4 | 1,  (1) << 4 | 2,
    (1) << 4 | 3,  (1) << 4 | 4,  (1) << 4 | 5,  (1) << 4 | 6,  (1) << 4 | 7,
    (1) << 4 | 8,  (1) << 4 | 9,  (1) << 4 | 10, (1) << 4 | 11, (2) << 4,
    (2) << 4 | 1,  (2) << 4 | 2,  (2) << 4 | 3,  (2) << 4 | 4,  (2) << 4 | 5,
    (2) << 4 | 6,  (2) << 4 | 7,  (2) << 4 | 8,  (2) << 4 | 9,  (2) << 4 | 10,
    (2) << 4 | 11, (3) << 4,      (3) << 4 | 1,  (3) << 4 | 2,  (3) << 4 | 3,
    (3) << 4 | 4,  (3) << 4 | 5,  (3) << 4 | 6,  (3) << 4 | 7,  (3) << 4 | 8,
    (3) << 4 | 9,  (3) << 4 | 10, (3) << 4 | 11, (4) << 4,      (4) << 4 | 1,
    (4) << 4 | 2,  (4) << 4 | 3,  (4) << 4 | 4,  (4) << 4 | 5,  (4) << 4 | 6,
    (4) << 4 | 7,  (4) << 4 | 8,  (4) << 4 | 9,  (4) << 4 | 10, (4) << 4 | 11,
    (5) << 4,      (5) << 4 | 1,  (5) << 4 | 2,  (5) << 4 | 3,  (5) << 4 | 4,
    (5) << 4 | 5,  (5) << 4 | 6,  (5) << 4 | 7,  (5) << 4 | 8,  (5) << 4 | 9,
    (5) << 4 | 10, (5) << 4 | 11, (6) << 4,      (6) << 4 | 1,  (6) << 4 | 2,
    (6) << 4 | 3,  (6) << 4 | 4,  (6) << 4 | 5,  (6) << 4 | 6,  (6) << 4 | 7,
    (6) << 4 | 8,  (6) << 4 | 9,  (6) << 4 | 10, (6) << 4 | 11, (7) << 4,
    (7) << 4 | 1,  (7) << 4 | 2,  (7) << 4 | 3,  (7) << 4 | 4,  (7) << 4 | 5,
    (7) << 4 | 6,  (7) << 4 | 7,  (7) << 4 | 8,  (7) << 4 | 9,  (7) << 4 | 10,
    (7) << 4 | 11, 8 << 4};

static const uint32_t INC_LIM[32] = {0xFFFFFFFFu,
                                     0x7FFFFFFFu,
                                     0x7FFFFFFFu / 2,
                                     0x7FFFFFFFu / 3,
                                     0x7FFFFFFFu / (4),
                                     0x7FFFFFFFu / (4 + 1),
                                     0x7FFFFFFFu / (4 + 2),
                                     0x7FFFFFFFu / (4 + 3),
                                     0x7FFFFFFFu / (8),
                                     0x7FFFFFFFu / (8 + 1),
                                     0x7FFFFFFFu / (8 + 2),
                                     0x7FFFFFFFu / (8 + 3),
                                     0x7FFFFFFFu / (12),
                                     0x7FFFFFFFu / (12 + 1),
                                     0x7FFFFFFFu / (12 + 2),
                                     0x7FFFFFFFu / (12 + 3),
                                     0x7FFFFFFFu / (16),
                                     0x7FFFFFFFu / (16 + 1),
                                     0x7FFFFFFFu / (16 + 2),
                                     0x7FFFFFFFu / (16 + 3),
                                     0x7FFFFFFFu / (20),
                                     0x7FFFFFFFu / (20 + 1),
                                     0x7FFFFFFFu / (20 + 2),
                                     0x7FFFFFFFu / (20 + 3),
                                     0x7FFFFFFFu / (24),
                                     0x7FFFFFFFu / (24 + 1),
                                     0x7FFFFFFFu / (24 + 2),
                                     0x7FFFFFFFu / (24 + 3),
                                     0x7FFFFFFFu / (28),
                                     0x7FFFFFFFu / (28 + 1),
                                     0x7FFFFFFFu / (28 + 2),
                                     0x7FFFFFFFu / (28 + 3)};

typedef struct {
  uint32_t inc, sh, r;
} rcp_t;

typedef struct {
  uint32_t ph;
  int32_t prev;
} osc_t;

typedef struct {
  int32_t a1, a2, a3, k;
} svf_coef_t;

typedef struct {
  int32_t ic1, ic2;
} svf_state_t;

enum {
  P_O1_WAVE,
  P_O1_PW,
  P_O1_LVL,
  P_NOISE,
  P_O2_WAVE,
  P_O2_PW,
  P_O2_LVL,
  P_O2_SEMI,
  P_SYNC,
  P_XMOD,
  P_ENV_O2,
  P_O2_FINE,
  P_CUTOFF,
  P_RESO,
  P_FENV_AMT,
  P_KEYTRK,
  P_FTYPE,
  P_DRIVE,
  P_VEL_FLT,
  P_VEL_AMP,
  P_FA,
  P_FD,
  P_FS,
  P_FR,
  P_AA,
  P_AD,
  P_AS,
  P_AR,
  P_LFO_SHAPE,
  P_LFO_RATE,
  P_LFO_DELAY,
  P_LFO_PHASE,
  P_LFO_PITCH,
  P_LFO_PWM,
  P_LFO_FLT,
  P_LFO_AMP,
  P_LFO_PITCH_TO,
  P_LFO_PWM_TO,
  P_MODE,
  P_UNI_N,
  P_UNI_DET,
  P_GLIDE,
  P_VINTAGE,
  P_SPREAD,
  P_BEND,
  P_VOLUME,
  P_MOD_DEPTH,
  P_ARP_MODE,
  P_ARP_DIV,
  P_ARP_OCT,
  P_ARP_GATE,
  P_FMORPH,
  P_CHO_RATE,
  P_CHO_DEPTH,
  P_CHO_MIX,
  P_DLY_TIME,
  P_DLY_FB,
  P_DLY_TONE,
  P_DLY_MIX,
  P_REV_SIZE,
  P_REV_DAMP,
  P_REV_PRE,
  P_REV_MIX,
  NPARAM
};

enum { W_OFF, W_SAW, W_PLS, W_BOTH };

enum { F_LP12, F_HP12, F_BP12, F_NOTCH, F_LP24, F_SEM, NFTYPE };

enum { M_POLY, M_DUAL, M_UNISON, M_MONO };

enum { L_TRI, L_SAW, L_RAMP, L_SQR, L_SH, L_RND };

enum { LP_SYNC, LP_FREE, LP_SPREAD };

enum { TO_O1, TO_O2, TO_BOTH };

enum { A_OFF, A_UP, A_DOWN, A_UPDN, A_RND, A_PLAY, NARPMODE };

enum { D_4, D_8, D_8T, D_16, D_16T, D_32, D_32T, NDIV };

enum { DD_16, DD_8T, DD_8, DD_8D, DD_4, DD_4D, DD_2, NDDIV };

static const uint8_t OBV_DEF[NPARAM] = {
    W_SAW, 64,  110, 0,       W_SAW,   64,     100,   24, 0,  0,       64,
    70,    80,  20,  90,      64,      F_LP24, 0,     40, 60, 20,      70,
    40,    60,  10,  70,      100,     60,     L_TRI, 70, 0,  LP_FREE, 0,
    0,     0,   0,   TO_BOTH, TO_BOTH, M_POLY, 4,     30, 0,  30,      60,
    2,     100, 0,   A_OFF,   D_16,    1,      64,    0,  40, 50,      0,
    DD_8D, 50,  70,  0,       80,      60,     20,    0,
};

static const int32_t OX_BIRD_L[13][2] = {
    {9644, 4390}, {8866, 3996}, {8098, 3593}, {7330, 3190}, {6527, 2822},
    {5726, 2453}, {4935, 2073}, {4110, 1726}, {3305, 1361}, {2480, 1015},
    {1649, 677},  {851, 311},   {0, 0}};

static const int32_t OX_BIRD_R[13][2] = {
    {-10269, 3618}, {-9444, 3272}, {-8620, 2926}, {-7781, 2600}, {-6941, 2275},
    {-6102, 1949},  {-5263, 1624}, {-4409, 1319}, {-3538, 1039}, {-2657, 771},
    {-1768, 517},   {-891, 244},   {0, 0}};

static_assert(8u == 8, "Oxbow ramp and table control rate must agree");

enum { E_IDLE, E_ATT, E_DEC, E_SUS, E_REL };

typedef struct {
  uint32_t stage;
  int32_t level;
} env_t;

typedef struct {
  osc_t o1, o2;
  rcp_t r1, r2;
  uint32_t noise;
  env_t fe;
  svf_state_t f1, f2;
  svf_coef_t c1, c2;
  uint32_t inc1, inc2, pw1, pw2;
  int32_t up3, up2, up1;
  int32_t amp0, amp1;
  int32_t drift;
  uint32_t lfo_blk;
  int32_t lfo_fade;
  int32_t c1_cut, c1_k, c2_cut;
  uint8_t note, vel, gate, fresh;
  uint8_t c1_ok, c2_ok;
} voice_t;

static const int16_t VINT_TUNE8[] = {-20, 12, 7, -15, 18, -9};

static const int16_t VINT_CUT8[] = {-300, 250, 120, -380, 60, 330};

static const int8_t VINT_ENV[] = {10, -6, 14, -12, 3, -16};

static const int32_t keytrk = 64, velflt = 40;

static const int32_t lfo_pd = 0, lfo_fd = 0, lfo_pwd = 0, lfo_ad = 0,
                     env_o2 = 0, uni_det8 = 0;

static const int32_t lvl1 = (110 * 110) >> 7, lvl2 = (100 * 100) >> 7, lvln = 0,
                     vint = 30;

static const int32_t sus_f = 40 * ((1 << 24) / 127), morph_q = 0;

static const uint32_t pw1_base = (64u << 25) + (1u << 24),
                      pw2_base = (64u << 25) + (1u << 24);

static const uint32_t lfo_dl = 0;

static const int32_t lfo_fstep = 32768;

typedef struct {
  int32_t fka, fkd, fkr;
  int32_t p8, p2, cut, velg;
  uint32_t spread, lfo_on, to1, to2, pw_to1, pw_to2;
} vblk_t;

typedef struct {
  uint32_t inc, pw, sh, r;
} otick_t;

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  voice_t vc[1];
  int last_voice;
  int osc_mode;
  uint32_t lfo_ph, lfo_rnd_a, lfo_rnd_b, rnd_state;
  uint32_t t_lfo_ph[((128 / 2) / (2 * (int)8u))],
      t_rnd_a[((128 / 2) / (2 * (int)8u))],
      t_rnd_b[((128 / 2) / (2 * (int)8u))];
  int32_t t_cut[((int)((128 / 2) / 8u))];
  int32_t cut_sm, reso_sm;
  int32_t fenv_amt, det_fine8;
  int32_t drive_q12, reso_k;
  uint32_t w1, w2, ftype, sync_on, xmod_q;
  volatile uint32_t obv_blocks;
  volatile uint32_t obv_vblk[1];
  volatile int32_t obv_venv[1];
  vblk_t block_cache[1];
  struct {
    int mode, detune;
    uint8_t note, vel, valid;
  } block_key[1];
  otick_t tk1[((128 / 2) / (2 * (int)8u))], tk2[((128 / 2) / (2 * (int)8u))];
  svf_coef_t tc1[((int)((128 / 2) / 8u))], tc2[((int)((128 / 2) / 8u))];
  int32_t tamp[((int)((128 / 2) / 8u)) + 1];
  int32_t bmix[(128 / 2)], bnoise[(128 / 2)];
  void defaults() {
    last_voice = 1;
    rnd_state = 0x9E3779B9u;
    cut_sm = -1;
  }
  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  inline int32_t clampi(int32_t v, int32_t lo, int32_t hi) {
    return v < lo ? lo : v > hi ? hi : v;
  }

  inline __attribute__((always_inline)) int32_t softclip(int32_t x) {
    int32_t a = x < 0 ? -x : x, i = a >> 8, y;
    if (i >= 256)
      y = TANH_Q15[256];
    else
      y = TANH_Q15[i] + (((TANH_Q15[i + 1] - TANH_Q15[i]) * (a & 255)) >> 8);
    return x < 0 ? -y : y;
  }

  inline uint32_t xorshift(uint32_t *st) {
    uint32_t s = *st;
    s ^= s << 13;
    s ^= s >> 17;
    s ^= s << 5;
    *st = s;
    return s;
  }

  inline int32_t obv_add_sat(int32_t a, int32_t b) {
    int64_t sum = (int64_t)a + b;
    return sum > 2147483647          ? 2147483647
           : sum < (-2147483647 - 1) ? (-2147483647 - 1)
                                     : (int32_t)sum;
  }

  inline __attribute__((always_inline)) int32_t mulq24(int32_t a, int32_t b) {
    int32_t ah = a >> 12;
    uint32_t al = (uint32_t)a & 4095u;
    int32_t bh = b >> 12;
    uint32_t bl = (uint32_t)b & 4095u;
    return ah * bh + ((ah * (int32_t)bl + (int32_t)(al * (uint32_t)bh)) >> 12) +
           (int32_t)((al * bl) >> 24);
  }

  inline __attribute__((always_inline)) int32_t bitlen(uint32_t x) {
    return x ? 32 - leading_zeroes(x) : 0;
  }

  inline __attribute__((always_inline)) uint32_t rcp30(uint32_t d) {
    const uint32_t j = (d >> 7) - 256u;
    uint32_t q =
        (uint32_t)((int32_t)RCP_Q[j] -
                   (((int32_t)RCP_S[j] * ((int32_t)(d & 127u) - 64)) >> 12));
    if (q * d > (1u << 30))
      q--;
    else if ((q + 1u) * d <= (1u << 30))
      q++;
    return q;
  }

  inline uint32_t div25(uint32_t d) {
    int32_t s;
    uint32_t q;
    if (d < 4096u)
      return (1u << 25) / d;
    s = bitlen(d) - 16;
    if (s <= 0)
      return rcp30(d << -s) >> (5 + s);
    q = rcp30(d >> s) >> (5 + s);
    if (q * d > (1u << 25))
      q--;
    else if ((q + 1u) * d <= (1u << 25))
      q++;
    return q;
  }

  inline uint32_t pitch_ratio(int32_t p8) {
    uint32_t os, r;
    int32_t o;
    p8 = clampi(p8, -48 * 256, 48 * 256);
    os = OCT_SEMI[(p8 >> 8) + 48];
    o = (int32_t)(os >> 4);
    r = (SEMI_Q16[os & 15u] * (FINE_Q16[p8 & 255] >> 1)) >> 15;
    return o >= 4 ? r << (o - 4) : r >> (4 - o);
  }

  inline uint32_t inc_scale(uint32_t inc, uint32_t ratio) {
    uint32_t rh = ratio >> 16, rl = ratio & 0xFFFFu, r;
    uint32_t lo = (inc >> 16) * rl + (((inc & 0xFFFFu) * rl) >> 16);
    if (inc > INC_LIM[rh])
      return 0x7FFFFFFFu;
    r = inc * rh + lo;
    return (int32_t)r < 0 ? 0x7FFFFFFFu : r;
  }

  inline void rcp_make(rcp_t *c, uint32_t inc) {
    int32_t sh = (int32_t)inc < 0 ? 16 : bitlen(inc) - 16;
    uint32_t d;
    if (sh < 0)
      sh = 0;
    c->inc = inc;
    c->sh = sh;
    d = inc >> sh;
    c->r = d >= 32768u ? rcp30(d) : d ? (1u << 30) / d : 0;
  }

  inline __attribute__((always_inline)) void rcp_track(rcp_t *c, uint32_t inc) {
    uint32_t d = inc > c->inc ? inc - c->inc : c->inc - inc;
    if (d > (c->inc >> 8))
      rcp_make(c, inc);
  }

  inline __attribute__((always_inline)) int32_t rcp_frac(rcp_t *c, uint32_t inc,
                                                         uint32_t x) {
    uint32_t t;
    rcp_track(c, inc);
    t = ((x >> c->sh) * c->r) >> 15;
    return t > 32767u ? 32767 : (int32_t)t;
  }

  inline __attribute__((always_inline)) int32_t rcp_at(uint32_t sh, uint32_t r,
                                                       uint32_t x) {
    uint32_t t = ((x >> sh) * r) >> 15;
    return t > 32767u ? 32767 : (int32_t)t;
  }

  inline __attribute__((always_inline)) int32_t osc_naive(uint32_t ph,
                                                          uint32_t pw,
                                                          uint32_t wave) {
    int32_t v = 0;
    if (wave & 1u)
      v = (int32_t)(ph >> 16) - 32768;
    if (wave & 2u)
      v += (int32_t)(ph >> 16) - (int32_t)((ph + pw) >> 16);
    return v;
  }

  inline __attribute__((always_inline)) void
  osc_event(int32_t *prev, int32_t *cur, int32_t h, int32_t t) {
    int32_t u = 32767 - t, h16 = h >> 4;
    *prev += (h16 * ((t * t) >> 15)) >> 12;
    *cur -= (h16 * ((u * u) >> 15)) >> 12;
  }

  inline __attribute__((always_inline)) int32_t
  osc_step(osc_t *o, uint32_t inc, uint32_t d, uint32_t pw, uint32_t wave,
           int32_t hw, int32_t reset_t, rcp_t *rc) {
    uint32_t old = o->ph, ph = old + inc;
    int32_t cur, out = o->prev;
    if (reset_t < 0 || !d) {
      cur = osc_naive(ph, pw, wave);
      if (ph < inc && d)
        osc_event(&out, &cur, hw, rcp_frac(rc, inc, ph));
      if ((wave & 2u) && ph + pw < inc && d)
        osc_event(&out, &cur, 65536, rcp_frac(rc, inc, ph + pw));
    } else {
      uint32_t at = ph - d * (uint32_t)reset_t;
      int32_t t;
      ph = d * (uint32_t)reset_t;
      cur = osc_naive(ph, pw, wave);
      if (at < old) {
        t = rcp_frac(rc, inc, at) + reset_t;
        osc_event(&out, &cur, hw, t > 32767 ? 32767 : t);
      }
      if ((wave & 2u) && at + pw < old + pw) {
        t = rcp_frac(rc, inc, at + pw) + reset_t;
        osc_event(&out, &cur, 65536, t > 32767 ? 32767 : t);
      }
      osc_event(&out, &cur, osc_naive(0, pw, wave) - osc_naive(at, pw, wave),
                reset_t);
    }
    o->ph = ph;
    o->prev = cur;
    return out;
  }

  inline __attribute__((always_inline)) int32_t svf_dot(int32_t a, int32_t x,
                                                        int32_t b, int32_t y) {
    if ((uint32_t)x + 65536u < 131072u && (uint32_t)y + 65536u < 131072u)
      return (a * x + b * y) >> 13;
    return a * (x >> 13) + b * (y >> 13) +
           ((a * (int32_t)((uint32_t)x & 8191u) +
             b * (int32_t)((uint32_t)y & 8191u)) >>
            13);
  }

  inline __attribute__((always_inline)) int32_t svf_damp(int32_t k, int32_t x) {
    if ((uint32_t)x + 131072u < 262144u)
      return (k * x) >> 12;
    return k * (x >> 12) + ((k * (int32_t)((uint32_t)x & 4095u)) >> 12);
  }

  inline void svf_coef(svf_coef_t *c, int32_t cut, int32_t k) {
    int32_t i, g, den;
    cut = clampi(cut, 0, 136 << 8);
    i = cut >> 8;
    g = SVF_G[i];
    if (i < 136)
      g += ((SVF_G[i + 1] - g) * (cut & 255)) >> 8;
    den = 4096 + ((g * (g + k)) >> 12);
    if (den < 64)
      den = 64;
    c->a1 = (int32_t)div25((uint32_t)den);
    c->a2 = (c->a1 * g) >> 12;
    c->a3 = (c->a2 * g) >> 12;
    c->k = k;
  }

  inline __attribute__((always_inline)) void
  svf_step(const svf_coef_t *c, svf_state_t *s, int32_t in, int32_t *lp,
           int32_t *bp, int32_t *hp) {
    int32_t v3 = in - s->ic2;
    int32_t v1 = svf_dot(c->a1, s->ic1, c->a2, v3);
    int32_t v2 = s->ic2 + svf_dot(c->a2, s->ic1, c->a3, v3);
    int32_t n1 = 2 * v1 - s->ic1;
    s->ic1 = (uint32_t)(n1 + 24576) < 49152u ? n1 : softclip(n1 >> 1) * 2;
    s->ic2 = clampi(2 * v2 - s->ic2, -131071, 131071);
    *lp = v2;
    *bp = v1;
    *hp = in - svf_damp(c->k, v1) - v2;
  }

  inline __attribute__((always_inline)) int32_t svf_lp(const svf_coef_t *c,
                                                       svf_state_t *s,
                                                       int32_t in) {
    int32_t v3 = in - s->ic2;
    int32_t v1 = svf_dot(c->a1, s->ic1, c->a2, v3);
    int32_t v2 = s->ic2 + svf_dot(c->a2, s->ic1, c->a3, v3);
    s->ic1 = clampi(2 * v1 - s->ic1, -131071, 131071);
    s->ic2 = clampi(2 * v2 - s->ic2, -131071, 131071);
    return v2;
  }

  unsigned ox_select(int32_t q, unsigned count) {
    return ((unsigned)(q < 0 ? 0 : q) >> 9) % count;
  }

  inline int32_t knob(const int16_t *k, int i) { return k[i] < 0 ? 0 : k[i]; }

  inline uint32_t phase_2x(uint32_t x) {
    return x > 0x3fffffffu ? 0x7fffffffu : x << 1;
  }

  int32_t lfo_shape(uint32_t ph, uint32_t shape, uint32_t ra, uint32_t rb) {
    int32_t p;
    switch (shape) {
    case L_TRI:
      p = (int32_t)((ph + 0x40000000u) >> 15);
      return p < 65536 ? p - 32768 : 98303 - p;
    case L_SAW:
      return 32767 - (int32_t)(ph >> 16);
    case L_RAMP:
      return (int32_t)(ph >> 16) - 32768;
    case L_SQR:
      return ph < 0x80000000u ? 32767 : -32767;
    case L_SH:
      return (int32_t)(rb >> 16) - 32768;
    default: {
      int32_t a = (int32_t)(ra >> 16) - 32768, b = (int32_t)(rb >> 16) - 32768;
      return a + mulq24(b - a, (int32_t)(ph >> 8));
    }
    }
  }

  void block_shared(const int16_t *k) {
    int32_t d, i;
    osc_mode = knob(k, 5) * 6 >> 15;
    int32_t cut_t = (knob(k, 0) * 127) >> 7;
    int32_t reso_t = (knob(k, 1) * 127) >> 7;
    fenv_amt = ((knob(k, 2) * 127) >> 15) - 64;
    det_fine8 = (knob(k, 3) * 128) >> 15;
    i = (int)ox_select(knob(k, 6), NFTYPE);
    ftype = (uint32_t)i;
    d = OBV_DEF[P_DRIVE] + ((knob(k, 7) * 127) >> 15);
    d = d > 127 ? 127 : d;
    drive_q12 = d ? 4096 + (d * d * 7 * 4096 / 16129) : 0;
    if (cut_sm < 0) {
      cut_sm = cut_t;
      reso_sm = reso_t;
    }
    for (i = 0; i < ((int)((128 / 2) / 8u)); i++) {
      cut_sm += (cut_t - cut_sm) >> 6;
      reso_sm += (reso_t - reso_sm) >> 6;
      t_cut[i] = cut_sm;
    }
    reso_k = 8192 - reso_sm * 8400 / (127 * 256);
    for (i = 0; i < ((int)((128 / 2) / 8u)); i++) {
      uint32_t old = lfo_ph;
      lfo_ph += LFO_INC[OBV_DEF[P_LFO_RATE]];
      if (lfo_ph < old) {
        lfo_rnd_a = lfo_rnd_b;
        lfo_rnd_b = xorshift(&rnd_state);
      }
      if (i & 1)
        continue;
      t_lfo_ph[i >> 1] = lfo_ph + (LFO_INC[OBV_DEF[P_LFO_RATE]] >> 1);
      t_rnd_a[i >> 1] = lfo_rnd_a;
      t_rnd_b[i >> 1] = lfo_rnd_b;
    }
    w1 = w2 = osc_mode == 1 ? W_PLS : osc_mode == 2 ? W_BOTH : W_SAW;
    sync_on = osc_mode == 3;
    xmod_q = osc_mode == 4 ? 96 : 0;
    obv_blocks++;
  }

  inline __attribute__((always_inline)) void env_tick(env_t *e, int32_t ka,
                                                      int32_t kd, int32_t sus,
                                                      int32_t kr,
                                                      int32_t gate) {
    if (gate && e->stage >= E_REL)
      e->stage = E_ATT;
    if (!gate && e->stage && e->stage < E_REL)
      e->stage = E_REL;
    switch (e->stage) {
    case E_ATT:
      e->level += mulq24((1 << 24) + ((1 << 24) * 3 >> 3) - e->level, ka);
      if (e->level >= (1 << 24)) {
        e->level = (1 << 24);
        e->stage = E_DEC;
      }
      break;
    case E_DEC:
      e->level += mulq24(sus - e->level, kd) - 16;
      if (e->level <= sus + 64) {
        e->level = sus;
        e->stage = E_SUS;
      }
      break;
    case E_SUS:
      e->level = sus;
      break;
    case E_REL:
      e->level += mulq24(-e->level, kr) - 32;
      if (e->level < 2048) {
        e->level = 0;
        e->stage = E_IDLE;
      }
      break;
    default:
      e->level = 0;
    }
  }

  inline int32_t env_k(uint32_t k, int32_t vt) {
    return (int32_t)k + (((int32_t)k * vt) >> 7);
  }

  inline uint32_t pw_mod(uint32_t base, int32_t mod) {
    const uint32_t lo = 1u << 26, hi = 0xFFFFFFFFu - (1u << 26);
    uint32_t m = (uint32_t)(mod < 0 ? -mod : mod) << 8;
    if (mod < 0)
      return base < lo + m ? lo : (base - m > hi ? hi : base - m);
    return base > hi - m ? hi : (base + m < lo ? lo : base + m);
  }

  uint8_t note_of(uint32_t inc) {
    uint32_t lo = 0, hi = 127;
    while (lo < hi) {
      uint32_t n = (lo + hi + 1u) >> 1;
      uint32_t i = INC_C9[n % 12u] >> (10u - n / 12u);
      if (inc >= i - (i >> 5))
        lo = n;
      else
        hi = n - 1u;
    }
    return (uint8_t)lo;
  }

  const vblk_t *voice_block(const voice_t *v, int card) {
    vblk_t *c = &block_cache[card];
    if (block_key[card].valid && block_key[card].mode == osc_mode &&
        block_key[card].detune == det_fine8 &&
        block_key[card].note == v->note && block_key[card].vel == v->vel)
      return c;
    block_key[card].mode = osc_mode;
    block_key[card].detune = det_fine8;
    block_key[card].note = v->note;
    block_key[card].vel = v->vel;
    block_key[card].valid = 1;
    const int32_t vt = (vint * VINT_ENV[card]) >> 7;
    c->fka = env_k(ENV_K_ATT[OBV_DEF[P_FA]], vt);
    c->fkd = env_k(ENV_K_DEC[OBV_DEF[P_FD]], vt);
    c->fkr = env_k(ENV_K_DEC[OBV_DEF[P_FR]], vt);
    c->p8 = (VINT_TUNE8[card] * vint) >> 7;
    c->p2 = ((int32_t)OBV_DEF[P_O2_SEMI] - 24) * 256 + det_fine8 + uni_det8 +
            (osc_mode == 5                    ? 7 * 256
             : osc_mode == 3 || osc_mode == 4 ? 12 * 256
                                              : 0);
    c->cut = 8 * 256 + ((int32_t)v->note - 60) * keytrk * 2 +
             (((int32_t)v->vel - 64) * velflt >> 1) +
             ((VINT_CUT8[card] * vint) >> 7);
    c->velg =
        32767 -
        ((((127 - (int32_t)v->vel) * (int32_t)OBV_DEF[P_VEL_AMP]) >> 7) * 258);
    c->spread = OBV_DEF[P_LFO_PHASE] == LP_SPREAD ? (uint32_t)card << 29 : 0;
    c->lfo_on = lfo_pd | lfo_pwd | lfo_fd | lfo_ad;
    c->to1 = OBV_DEF[P_LFO_PITCH_TO] != TO_O2;
    c->to2 = OBV_DEF[P_LFO_PITCH_TO] != TO_O1;
    c->pw_to1 = OBV_DEF[P_LFO_PWM_TO] != TO_O2;
    c->pw_to2 = OBV_DEF[P_LFO_PWM_TO] != TO_O1;
    return c;
  }

  inline void voice_coef(voice_t *v, int32_t cut, svf_coef_t *c1,
                         svf_coef_t *c2) {
    if (!v->c1_ok || cut != v->c1_cut || reso_k != v->c1_k) {
      svf_coef(&v->c1, cut, reso_k);
      v->c1_cut = cut;
      v->c1_k = reso_k;
      v->c1_ok = 1;
    }
    *c1 = v->c1;
    if (ftype == F_LP24) {
      if (!v->c2_ok || cut != v->c2_cut) {
        svf_coef(&v->c2, cut, 5793);
        v->c2_cut = cut;
        v->c2_ok = 1;
      }
      *c2 = v->c2;
    }
  }

  void voice_tick(voice_t *v, const vblk_t *c, int t, uint32_t base,
                  int32_t *amp, svf_coef_t *c1, svf_coef_t *c2,
                  int32_t *noise) {
    int32_t lfo = 0, p8, p1, p2, cut, fenv, tmp, trem, lfo_to, f1, a1;
    if (c->lfo_on)
      lfo = lfo_shape(t_lfo_ph[t] + c->spread, OBV_DEF[P_LFO_SHAPE], t_rnd_a[t],
                      t_rnd_b[t]);
    for (int s = 0; s < 2; s++)
      if (v->lfo_fade < 32767) {
        if (v->lfo_blk >= lfo_dl) {
          v->lfo_fade += lfo_fstep;
          if (v->lfo_fade > 32767)
            v->lfo_fade = 32767;
        }
        v->lfo_blk++;
      }
    lfo = (lfo * v->lfo_fade) >> 15;
    env_tick(&v->fe, c->fka, c->fkd, sus_f, c->fkr, v->gate);
    f1 = v->fe.level;
    env_tick(&v->fe, c->fka, c->fkd, sus_f, c->fkr, v->gate);
    a1 = (1 << 24);
    fenv = ((f1 >> 1) + (v->fe.level >> 1)) >> 9;
    for (int s = 0; s < 2; s++) {
      v->drift += (int32_t)(xorshift(&v->noise) & 31u) - 16;
      v->drift -= v->drift >> 11;
      if (noise) {
        uint32_t ns = v->noise;
        for (int i = 0; i < (int)8u; i++)
          *noise++ = ((int32_t)(xorshift(&ns) >> 16) - 32768) * lvln;
        v->noise = ns;
      }
    }
    p8 = c->p8 + ((v->drift * vint) >> 10);
    lfo_to = (lfo * lfo_pd * 24) >> 15;
    p1 = p8 + (c->to1 ? lfo_to : 0);
    p2 = p8 + (c->to2 ? lfo_to : 0) + c->p2 + ((fenv * env_o2 * 97) >> 15);
    v->inc1 = inc_scale(base, pitch_ratio(p1));
    v->inc2 = inc_scale(base, pitch_ratio(p2));
    tmp = lfo * lfo_pwd;
    v->pw1 = pw_mod(pw1_base, c->pw_to1 ? tmp : 0);
    v->pw2 = pw_mod(pw2_base, c->pw_to2 ? tmp : 0);
    cut = c->cut + ((lfo * lfo_fd * 97) >> 15);
    voice_coef(v, cut + t_cut[2 * t] + (((f1 >> 9) * fenv_amt * 390) >> 15), c1,
               c2);
    voice_coef(v,
               cut + t_cut[2 * t + 1] +
                   (((v->fe.level >> 9) * fenv_amt * 390) >> 15),
               c1 + 1, c2 + 1);
    rcp_track(&v->r1, v->inc1);
    rcp_track(&v->r2, v->inc2);
    trem = 32767 - (((32767 - lfo) * lfo_ad) >> 8);
    amp[0] = (((a1 >> 9) * c->velg >> 15) * trem) >> 15;
    amp[1] = (((a1 >> 9) * c->velg >> 15) * trem) >> 15;
  }

  inline __attribute__((always_inline)) void
  osc_pass(osc_t *o, const otick_t *tk, uint32_t wave, int32_t hw, int32_t lvl,
           int32_t *mix, int first) {
    uint32_t ph = o->ph;
    int32_t prev = o->prev;
    for (int t = 0; t < ((128 / 2) / (2 * (int)8u));
         t++, tk++, mix += (2 * (int)8u)) {
      const uint32_t inc = tk->inc, pw = tk->pw, sh = tk->sh, r = tk->r;
      const int ev = (inc >> 15) != 0;
      for (int i = 0; i < (2 * (int)8u); i++) {
        int32_t cur, out = prev;
        ph += inc;
        cur = osc_naive(ph, pw, wave);
        if (ev) {
          if (ph < inc)
            osc_event(&out, &cur, hw, rcp_at(sh, r, ph));
          if ((wave & 2u) && ph + pw < inc)
            osc_event(&out, &cur, 65536, rcp_at(sh, r, ph + pw));
        }
        prev = cur;
        mix[i] = first ? out * lvl : mix[i] + out * lvl;
      }
    }
    o->ph = ph;
    o->prev = prev;
  }

  void osc_pass_w(osc_t *o, const otick_t *tk, uint32_t wave, int32_t hw,
                  int32_t lvl, int32_t *mix, int first) {
    switch (wave) {
    case W_SAW:
      if (first)
        osc_pass(o, tk, W_SAW, hw, lvl, mix, 1);
      else
        osc_pass(o, tk, W_SAW, hw, lvl, mix, 0);
      break;
    case W_PLS:
      if (first)
        osc_pass(o, tk, W_PLS, hw, lvl, mix, 1);
      else
        osc_pass(o, tk, W_PLS, hw, lvl, mix, 0);
      break;
    case W_BOTH:
      if (first)
        osc_pass(o, tk, W_BOTH, hw, lvl, mix, 1);
      else
        osc_pass(o, tk, W_BOTH, hw, lvl, mix, 0);
      break;
    default:
      if (first)
        osc_pass(o, tk, W_OFF, hw, lvl, mix, 1);
      else
        osc_pass(o, tk, W_OFF, hw, lvl, mix, 0);
      break;
    }
  }

  void osc_coupled(voice_t *v, int32_t *mix, int32_t hw1, int32_t hw2) {
    uint32_t inc1 = v->inc1, inc2 = v->inc2, pw1 = v->pw1, pw2 = v->pw2,
             d1 = inc1 >> 15, d2 = inc2 >> 15;
    uint32_t xq = xmod_q, sy = sync_on, ww1 = w1, ww2 = w2;
    int32_t o2n = v->o2.prev, l1 = lvl1, l2 = lvl2;
    for (int i = 0; i < (2 * (int)8u); i++) {
      int32_t s1, s2, rt = -1;
      uint32_t ie = inc1;
      if (xq) {
        int32_t m = (int32_t)(inc1 >> 8 > 0x200000u ? 0x200000u : inc1 >> 8) *
                    (((o2n >> 8) * (int32_t)xq) >> 6);
        m = obv_add_sat(m, (int32_t)inc1);
        ie = m < (int32_t)(inc1 >> 4) ? inc1 >> 4 : (uint32_t)m;
        d1 = ie >> 15;
      }
      s1 = osc_step(&v->o1, ie, d1, pw1, ww1, hw1, -1, &v->r1);
      if (sy && v->o1.ph < ie && d1)
        rt = rcp_frac(&v->r1, ie, v->o1.ph);
      s2 = osc_step(&v->o2, inc2, d2, pw2, ww2, hw2, rt, &v->r2);
      o2n = v->o2.prev;
      mix[i] = s1 * l1 + s2 * l2;
    }
  }

  inline __attribute__((always_inline)) void
  xmod_osc1(voice_t *v, int32_t *mix, const int32_t *o2c, int32_t o2n,
            uint32_t wave, int32_t hw) {
    const uint32_t inc1 = v->inc1, pw1 = v->pw1, xq = xmod_q,
                   lim = inc1 >> 8 > 0x200000u ? 0x200000u : inc1 >> 8;
    const int32_t l1 = lvl1;
    for (int i = 0; i < (2 * (int)8u); i++) {
      int32_t m = (int32_t)lim * (((o2n >> 8) * (int32_t)xq) >> 6), s1;
      uint32_t ie;
      m = obv_add_sat(m, (int32_t)inc1);
      ie = m < (int32_t)(inc1 >> 4) ? inc1 >> 4 : (uint32_t)m;
      s1 = osc_step(&v->o1, ie, ie >> 15, pw1, wave, hw, -1, &v->r1);
      mix[i] += s1 * l1;
      o2n = o2c[i];
    }
  }

  void osc_xmod(voice_t *v, int32_t *mix, int32_t hw1, int32_t hw2) {
    int32_t o2c[(2 * (int)8u)];
    const int32_t o2n = v->o2.prev;
    uint32_t ph = v->o2.ph;
    int32_t prev = v->o2.prev;
    const uint32_t inc = v->inc2, pw = v->pw2, sh = v->r2.sh, r = v->r2.r,
                   wave = w2;
    const int32_t l2 = lvl2, ev = (inc >> 15) != 0;
    for (int i = 0; i < (2 * (int)8u); i++) {
      int32_t cur, out = prev;
      ph += inc;
      cur = osc_naive(ph, pw, wave);
      if (ev) {
        if (ph < inc)
          osc_event(&out, &cur, hw2, rcp_at(sh, r, ph));
        if ((wave & 2u) && ph + pw < inc)
          osc_event(&out, &cur, 65536, rcp_at(sh, r, ph + pw));
      }
      prev = cur;
      o2c[i] = cur;
      mix[i] = out * l2;
    }
    v->o2.ph = ph;
    v->o2.prev = prev;
    switch (w1) {
    case W_SAW:
      xmod_osc1(v, mix, o2c, o2n, W_SAW, hw1);
      break;
    case W_PLS:
      xmod_osc1(v, mix, o2c, o2n, W_PLS, hw1);
      break;
    case W_BOTH:
      xmod_osc1(v, mix, o2c, o2n, W_BOTH, hw1);
      break;
    default:
      xmod_osc1(v, mix, o2c, o2n, W_OFF, hw1);
      break;
    }
  }

  void drive_pass(int32_t *mix) {
    const int32_t drv = drive_q12;
    if (drv)
      for (int i = 0; i < (128 / 2); i++)
        mix[i] = softclip((((mix[i] >> 7) >> 3) * drv) >> 9);
    else
      for (int i = 0; i < (128 / 2); i++)
        mix[i] = clampi(mix[i] >> 7, -65535, 65535);
  }

  inline __attribute__((always_inline)) void
  filter_pass(svf_state_t *f, int32_t *mix, uint32_t ft) {
    int32_t ic1 = f->ic1, ic2 = f->ic2;
    const int32_t mq = morph_q;
    const svf_coef_t *c = tc1;
    for (int t = 0; t < ((int)((128 / 2) / 8u)); t++, c++, mix += 8u) {
      const int32_t a1 = c->a1, a2 = c->a2, a3 = c->a3, k = c->k;
      for (int i = 0; i < (int)8u; i++) {
        int32_t x = mix[i], v1, v2, v3, n1, y;
        v3 = x - ic2;
        v1 = svf_dot(a1, ic1, a2, v3);
        v2 = ic2 + svf_dot(a2, ic1, a3, v3);
        n1 = 2 * v1 - ic1;
        ic1 = (uint32_t)(n1 + 24576) < 49152u ? n1 : softclip(n1 >> 1) * 2;
        ic2 = clampi(2 * v2 - ic2, -131071, 131071);
        switch (ft) {
        case F_HP12:
          y = x - svf_damp(k, v1) - v2;
          break;
        case F_BP12:
          y = v1;
          break;
        case F_NOTCH:
          y = x - svf_damp(k, v1);
          break;
        case F_SEM:
          y = v2 + (((x - svf_damp(k, v1) - v2 - v2) * mq) >> 7);
          break;
        default:
          y = v2;
          break;
        }
        mix[i] = y;
      }
    }
    f->ic1 = ic1;
    f->ic2 = ic2;
  }

  void lp2_pass(svf_state_t *f, int32_t *mix) {
    int32_t jc1 = f->ic1, jc2 = f->ic2;
    const svf_coef_t *c = tc2;
    for (int t = 0; t < ((int)((128 / 2) / 8u)); t++, c++, mix += 8u) {
      const int32_t b1 = c->a1, b2 = c->a2, b3 = c->a3;
      for (int i = 0; i < (int)8u; i++) {
        int32_t v3 = mix[i] - jc2, n1, y;
        n1 = svf_dot(b1, jc1, b2, v3);
        y = jc2 + svf_dot(b2, jc1, b3, v3);
        jc1 = clampi(2 * n1 - jc1, -131071, 131071);
        jc2 = clampi(2 * y - jc2, -131071, 131071);
        mix[i] = y;
      }
    }
    f->ic1 = jc1;
    f->ic2 = jc2;
  }

  void amp_pass(const int32_t *mix, int16_t *out) {
    for (int t = 0; t < ((int)((128 / 2) / 8u)); t++, mix += 8u, out += 8u) {
      const int32_t amp0 = tamp[t], da = tamp[t + 1] - tamp[t];
      for (int i = 0; i < (int)8u; i++) {
        int32_t y = clampi(mix[i], -65000, 65000), amp = amp0 + ((da * i) >> 3);
        y = (y * amp) >> 16;
        out[i] = op1_sat16((y * 20000) >> 15);
      }
    }
  }

  void voice_block_samples(voice_t *v, int vi, int16_t *out) {
    const uint32_t ww1 = w1, ww2 = w2, coupled = xmod_q || sync_on;
    const int32_t hw1 = (ww1 & 1u ? -65536 : 0) + (ww1 & 2u ? -65536 : 0);
    const int32_t hw2 = (ww2 & 1u ? -65536 : 0) + (ww2 & 2u ? -65536 : 0);
    const vblk_t *c = voice_block(v, vi);
    tamp[0] = v->amp1;
    for (int t = 0; t < ((128 / 2) / (2 * (int)8u)); t++) {
      voice_tick(v, c, t,
                 phase_2x((uint32_t)pitch_input[t * (2 * (int)8u) * 2]),
                 tamp + 2 * t + 1, tc1 + 2 * t, tc2 + 2 * t,
                 lvln ? bnoise + t * (2 * (int)8u) : 0);
      tk1[t].inc = v->inc1;
      tk1[t].pw = v->pw1;
      tk1[t].sh = v->r1.sh;
      tk1[t].r = v->r1.r;
      tk2[t].inc = v->inc2;
      tk2[t].pw = v->pw2;
      tk2[t].sh = v->r2.sh;
      tk2[t].r = v->r2.r;
      if (sync_on)
        osc_coupled(v, bmix + t * (2 * (int)8u), hw1, hw2);
      else if (coupled)
        osc_xmod(v, bmix + t * (2 * (int)8u), hw1, hw2);
    }
    v->amp1 = tamp[((int)((128 / 2) / 8u))];
    if (!coupled) {
      osc_pass_w(&v->o1, tk1, ww1, hw1, lvl1, bmix, 1);
      osc_pass_w(&v->o2, tk2, ww2, hw2, lvl2, bmix, 0);
    }
    if (lvln)
      for (int i = 0; i < (128 / 2); i++)
        bmix[i] += bnoise[i];
    drive_pass(bmix);
    switch (ftype) {
    case F_HP12:
      filter_pass(&v->f1, bmix, F_HP12);
      break;
    case F_BP12:
      filter_pass(&v->f1, bmix, F_BP12);
      break;
    case F_NOTCH:
      filter_pass(&v->f1, bmix, F_NOTCH);
      break;
    case F_SEM:
      filter_pass(&v->f1, bmix, F_SEM);
      break;
    default:
      filter_pass(&v->f1, bmix, F_LP12);
      break;
    }
    if (ftype == F_LP24)
      lp2_pass(&v->f2, bmix);
    amp_pass(bmix, out);
  }

  void obv_note_on(void *self, const int16_t *knobs) {
    const int vi = 0;
    voice_t *v = &vc[vi];
    int i, alone = 1;
    (void)knobs;
    for (i = 0; i < 1; i++)
      if (i != vi && vc[i].gate)
        alone = 0;
    if (!v->noise)
      v->noise = 0x9E3779B9u ^ ((uint32_t)(vi + 1) * 0x85EBCA6Bu);
    v->gate = 1;
    v->fresh = 1;
    v->fe.stage = E_ATT;
    v->lfo_blk = 0;
    v->lfo_fade = OBV_DEF[P_LFO_DELAY] ? 0 : 32767;
    if (alone && OBV_DEF[P_LFO_PHASE] == LP_SYNC)
      lfo_ph = 0;
  }

  void obv_note_off(void *self) { vc[0].gate = 0; }

  void obv_render(void *self, int16_t *out, const int16_t *knobs) {
    const int vi = 0;
    voice_t *v = &vc[vi];
    if (vi <= last_voice)
      block_shared(knobs);
    last_voice = vi;
    const uint32_t base = (uint32_t)pitch_input[0];
    if (v->fresh) {
      v->fresh = 0;
      v->note = note_of(base);
      v->vel = (uint8_t)(32767 / 258);
    }
    int16_t half[(128 / 2)];
    voice_block_samples(v, vi, half);
    int32_t h3 = v->up3, h2 = v->up2, h1 = v->up1;
    for (int i = 0; i < (128 / 2); i++) {
      const int32_t y = half[i];
      out[2 * i] = (int16_t)h2;
      out[2 * i + 1] = op1_sat16((-h3 + 9 * h2 + 9 * h1 - y) >> 4);
      h3 = h2;
      h2 = h1;
      h1 = y;
    }
    v->up3 = h3;
    v->up2 = h2;
    v->up1 = h1;
    obv_vblk[vi] = obv_blocks;
    obv_venv[vi] = v->fe.level >> 9;
  }
};
} // namespace port
} // namespace hkoxbow
