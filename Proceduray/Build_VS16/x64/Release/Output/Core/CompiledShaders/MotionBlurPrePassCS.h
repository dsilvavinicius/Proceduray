#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- -------------- ------
// ColorBuffer                       texture  float3          2d             t0      1 
// VelocityBuffer                    texture    uint          2d             t1      1 
// PrepBuffer                            UAV  float4          2d             u0      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// no Input
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// no Output
cs_5_0
dcl_globalFlags refactoringAllowed
dcl_resource_texture2d (float,float,float,float) t0
dcl_resource_texture2d (uint,uint,uint,uint) t1
dcl_uav_typed_texture2d (float,float,float,float) u0
dcl_input vThreadID.xy
dcl_temps 4
dcl_thread_group 8, 8, 1
imad r0.xyzw, vThreadID.xyxy, l(2, 2, 2, 2), l(0, 1, 1, 0)
mov r1.xy, r0.zwzz
mov r1.zw, l(0,0,0,0)
ld_indexable(texture2d)(uint,uint,uint,uint) r2.x, r1.xyww, t1.xyzw
ld_indexable(texture2d)(float,float,float,float) r1.yzw, r1.xyzw, t0.wxyz
bfi r2.y, l(9), l(4), r2.x, l(0)
ubfe r2.xzw, l(1, 0, 10, 1), l(9, 0, 10, 19), r2.xxxx
imad r2.x, r2.x, l(0x00008000), r2.y
f16tof32 r2.x, r2.x
bfi r2.z, l(9), l(4), r2.z, l(0)
imad r2.z, r2.w, l(0x00008000), r2.z
f16tof32 r2.z, r2.z
mul r2.xy, r2.xzxx, l(32768.000000, 32768.000000, 0.000000, 0.000000)
dp2 r2.x, r2.xyxx, r2.xyxx
sqrt r2.x, r2.x
mul r2.x, r2.x, l(8.000000)
min r2.x, r2.x, l(1.000000)
mov r1.x, l(1.000000)
mul r1.xyzw, r1.xyzw, r2.xxxx
ishl r2.xy, vThreadID.xyxx, l(1, 1, 0, 0)
mov r2.zw, l(0,0,0,0)
ld_indexable(texture2d)(uint,uint,uint,uint) r3.x, r2.xyww, t1.xyzw
ld_indexable(texture2d)(float,float,float,float) r2.yzw, r2.xyzw, t0.wxyz
bfi r3.y, l(9), l(4), r3.x, l(0)
ubfe r3.xzw, l(1, 0, 10, 1), l(9, 0, 10, 19), r3.xxxx
imad r3.x, r3.x, l(0x00008000), r3.y
f16tof32 r3.x, r3.x
bfi r3.z, l(9), l(4), r3.z, l(0)
imad r3.z, r3.w, l(0x00008000), r3.z
f16tof32 r3.z, r3.z
mul r3.xy, r3.xzxx, l(32768.000000, 32768.000000, 0.000000, 0.000000)
dp2 r3.x, r3.xyxx, r3.xyxx
sqrt r3.x, r3.x
mul r3.x, r3.x, l(8.000000)
min r3.x, r3.x, l(1.000000)
mov r2.x, l(1.000000)
mad r1.xyzw, r2.xyzw, r3.xxxx, r1.xyzw
mov r0.zw, l(0,0,0,0)
ld_indexable(texture2d)(uint,uint,uint,uint) r2.x, r0.xyww, t1.xyzw
ld_indexable(texture2d)(float,float,float,float) r0.yzw, r0.xyzw, t0.wxyz
bfi r2.y, l(9), l(4), r2.x, l(0)
ubfe r2.xzw, l(1, 0, 10, 1), l(9, 0, 10, 19), r2.xxxx
imad r2.x, r2.x, l(0x00008000), r2.y
f16tof32 r2.x, r2.x
bfi r2.z, l(9), l(4), r2.z, l(0)
imad r2.z, r2.w, l(0x00008000), r2.z
f16tof32 r2.z, r2.z
mul r2.xy, r2.xzxx, l(32768.000000, 32768.000000, 0.000000, 0.000000)
dp2 r2.x, r2.xyxx, r2.xyxx
sqrt r2.x, r2.x
mul r2.x, r2.x, l(8.000000)
min r2.x, r2.x, l(1.000000)
mov r0.x, l(1.000000)
mad r0.xyzw, r0.xyzw, r2.xxxx, r1.xyzw
imad r1.xy, vThreadID.xyxx, l(2, 2, 0, 0), l(1, 1, 0, 0)
mov r1.zw, l(0,0,0,0)
ld_indexable(texture2d)(uint,uint,uint,uint) r2.x, r1.xyww, t1.xyzw
ld_indexable(texture2d)(float,float,float,float) r1.yzw, r1.xyzw, t0.wxyz
bfi r2.y, l(9), l(4), r2.x, l(0)
ubfe r2.xzw, l(1, 0, 10, 1), l(9, 0, 10, 19), r2.xxxx
imad r2.x, r2.x, l(0x00008000), r2.y
f16tof32 r2.x, r2.x
bfi r2.z, l(9), l(4), r2.z, l(0)
imad r2.z, r2.w, l(0x00008000), r2.z
f16tof32 r2.z, r2.z
mul r2.xy, r2.xzxx, l(32768.000000, 32768.000000, 0.000000, 0.000000)
dp2 r2.x, r2.xyxx, r2.xyxx
sqrt r2.x, r2.x
mul r2.x, r2.x, l(8.000000)
min r2.x, r2.x, l(1.000000)
mov r1.x, l(1.000000)
mad r0.xyzw, r1.xyzw, r2.xxxx, r0.xyzw
add r0.x, r0.x, l(0.000100)
div r1.xyz, r0.yzwy, r0.xxxx
mul r0.x, r0.x, l(0.750000)
round_ni r0.w, r0.x
mul r0.xyz, r0.wwww, l(0.333333, 0.333333, 0.333333, 0.000000)
mov r1.w, l(0.333333)
mul r0.xyzw, r0.xyzw, r1.xyzw
store_uav_typed u0.xyzw, vThreadID.xyyy, r0.xyzw
ret 
// Approximately 81 instruction slots used
#endif

const BYTE g_pMotionBlurPrePassCS[] =
{
     68,  88,  66,  67, 130, 183, 
    239,  20, 140,  92, 173, 215, 
    233, 112, 131, 161, 236, 164, 
    209, 152,   1,   0,   0,   0, 
    116,  13,   0,   0,   6,   0, 
      0,   0,  56,   0,   0,   0, 
     44,   1,   0,   0,  60,   1, 
      0,   0,  76,   1,   0,   0, 
    252,  11,   0,   0, 152,  12, 
      0,   0,  82,  68,  69,  70, 
    236,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,  60,   0, 
      0,   0,   0,   5,  83,  67, 
      0,   1,   0,   0, 194,   0, 
      0,   0,  82,  68,  49,  49, 
     60,   0,   0,   0,  24,   0, 
      0,   0,  32,   0,   0,   0, 
     40,   0,   0,   0,  36,   0, 
      0,   0,  12,   0,   0,   0, 
      0,   0,   0,   0, 156,   0, 
      0,   0,   2,   0,   0,   0, 
      5,   0,   0,   0,   4,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   9,   0,   0,   0, 
    168,   0,   0,   0,   2,   0, 
      0,   0,   4,   0,   0,   0, 
      4,   0,   0,   0, 255, 255, 
    255, 255,   1,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0, 183,   0,   0,   0, 
      4,   0,   0,   0,   5,   0, 
      0,   0,   4,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     13,   0,   0,   0,  67, 111, 
    108, 111, 114,  66, 117, 102, 
    102, 101, 114,   0,  86, 101, 
    108, 111,  99, 105, 116, 121, 
     66, 117, 102, 102, 101, 114, 
      0,  80, 114, 101, 112,  66, 
    117, 102, 102, 101, 114,   0, 
     77, 105,  99, 114, 111, 115, 
    111, 102, 116,  32,  40,  82, 
     41,  32,  72,  76,  83,  76, 
     32,  83, 104,  97, 100, 101, 
    114,  32,  67, 111, 109, 112, 
    105, 108, 101, 114,  32,  49, 
     48,  46,  49,   0, 171, 171, 
     73,  83,  71,  78,   8,   0, 
      0,   0,   0,   0,   0,   0, 
      8,   0,   0,   0,  79,  83, 
     71,  78,   8,   0,   0,   0, 
      0,   0,   0,   0,   8,   0, 
      0,   0,  83,  72,  69,  88, 
    168,  10,   0,   0,  80,   0, 
      5,   0, 170,   2,   0,   0, 
    106,   8,   0,   1,  88,  24, 
      0,   4,   0, 112,  16,   0, 
      0,   0,   0,   0,  85,  85, 
      0,   0,  88,  24,   0,   4, 
      0, 112,  16,   0,   1,   0, 
      0,   0,  68,  68,   0,   0, 
    156,  24,   0,   4,   0, 224, 
     17,   0,   0,   0,   0,   0, 
     85,  85,   0,   0,  95,   0, 
      0,   2,  50,   0,   2,   0, 
    104,   0,   0,   2,   4,   0, 
      0,   0, 155,   0,   0,   4, 
      8,   0,   0,   0,   8,   0, 
      0,   0,   1,   0,   0,   0, 
     35,   0,   0,  14, 242,   0, 
     16,   0,   0,   0,   0,   0, 
     70,   4,   2,   0,   2,  64, 
      0,   0,   2,   0,   0,   0, 
      2,   0,   0,   0,   2,   0, 
      0,   0,   2,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,  54,   0,   0,   5, 
     50,   0,  16,   0,   1,   0, 
      0,   0, 230,  10,  16,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   8, 194,   0,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     45,   0,   0, 137, 194,   0, 
      0, 128,   3,  17,  17,   0, 
     18,   0,  16,   0,   2,   0, 
      0,   0,  70,  15,  16,   0, 
      1,   0,   0,   0,  70, 126, 
     16,   0,   1,   0,   0,   0, 
     45,   0,   0, 137, 194,   0, 
      0, 128,  67,  85,  21,   0, 
    226,   0,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  54, 121, 
     16,   0,   0,   0,   0,   0, 
    140,   0,   0,  11,  34,   0, 
     16,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   9,   0, 
      0,   0,   1,  64,   0,   0, 
      4,   0,   0,   0,  10,   0, 
     16,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0, 138,   0,   0,  15, 
    210,   0,  16,   0,   2,   0, 
      0,   0,   2,  64,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,  10,   0,   0,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,   9,   0,   0,   0, 
      0,   0,   0,   0,  10,   0, 
      0,   0,  19,   0,   0,   0, 
      6,   0,  16,   0,   2,   0, 
      0,   0,  35,   0,   0,   9, 
     18,   0,  16,   0,   2,   0, 
      0,   0,  10,   0,  16,   0, 
      2,   0,   0,   0,   1,  64, 
      0,   0,   0, 128,   0,   0, 
     26,   0,  16,   0,   2,   0, 
      0,   0, 131,   0,   0,   5, 
     18,   0,  16,   0,   2,   0, 
      0,   0,  10,   0,  16,   0, 
      2,   0,   0,   0, 140,   0, 
      0,  11,  66,   0,  16,   0, 
      2,   0,   0,   0,   1,  64, 
      0,   0,   9,   0,   0,   0, 
      1,  64,   0,   0,   4,   0, 
      0,   0,  42,   0,  16,   0, 
      2,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     35,   0,   0,   9,  66,   0, 
     16,   0,   2,   0,   0,   0, 
     58,   0,  16,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
      0, 128,   0,   0,  42,   0, 
     16,   0,   2,   0,   0,   0, 
    131,   0,   0,   5,  66,   0, 
     16,   0,   2,   0,   0,   0, 
     42,   0,  16,   0,   2,   0, 
      0,   0,  56,   0,   0,  10, 
     50,   0,  16,   0,   2,   0, 
      0,   0, 134,   0,  16,   0, 
      2,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,  71, 
      0,   0,   0,  71,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     15,   0,   0,   7,  18,   0, 
     16,   0,   2,   0,   0,   0, 
     70,   0,  16,   0,   2,   0, 
      0,   0,  70,   0,  16,   0, 
      2,   0,   0,   0,  75,   0, 
      0,   5,  18,   0,  16,   0, 
      2,   0,   0,   0,  10,   0, 
     16,   0,   2,   0,   0,   0, 
     56,   0,   0,   7,  18,   0, 
     16,   0,   2,   0,   0,   0, 
     10,   0,  16,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,  65,  51,   0, 
      0,   7,  18,   0,  16,   0, 
      2,   0,   0,   0,  10,   0, 
     16,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
    128,  63,  54,   0,   0,   5, 
     18,   0,  16,   0,   1,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0, 128,  63,  56,   0, 
      0,   7, 242,   0,  16,   0, 
      1,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
      6,   0,  16,   0,   2,   0, 
      0,   0,  41,   0,   0,   9, 
     50,   0,  16,   0,   2,   0, 
      0,   0,  70,   0,   2,   0, 
      2,  64,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  54,   0,   0,   8, 
    194,   0,  16,   0,   2,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  45,   0, 
      0, 137, 194,   0,   0, 128, 
      3,  17,  17,   0,  18,   0, 
     16,   0,   3,   0,   0,   0, 
     70,  15,  16,   0,   2,   0, 
      0,   0,  70, 126,  16,   0, 
      1,   0,   0,   0,  45,   0, 
      0, 137, 194,   0,   0, 128, 
     67,  85,  21,   0, 226,   0, 
     16,   0,   2,   0,   0,   0, 
     70,  14,  16,   0,   2,   0, 
      0,   0,  54, 121,  16,   0, 
      0,   0,   0,   0, 140,   0, 
      0,  11,  34,   0,  16,   0, 
      3,   0,   0,   0,   1,  64, 
      0,   0,   9,   0,   0,   0, 
      1,  64,   0,   0,   4,   0, 
      0,   0,  10,   0,  16,   0, 
      3,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
    138,   0,   0,  15, 210,   0, 
     16,   0,   3,   0,   0,   0, 
      2,  64,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
     10,   0,   0,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
      9,   0,   0,   0,   0,   0, 
      0,   0,  10,   0,   0,   0, 
     19,   0,   0,   0,   6,   0, 
     16,   0,   3,   0,   0,   0, 
     35,   0,   0,   9,  18,   0, 
     16,   0,   3,   0,   0,   0, 
     10,   0,  16,   0,   3,   0, 
      0,   0,   1,  64,   0,   0, 
      0, 128,   0,   0,  26,   0, 
     16,   0,   3,   0,   0,   0, 
    131,   0,   0,   5,  18,   0, 
     16,   0,   3,   0,   0,   0, 
     10,   0,  16,   0,   3,   0, 
      0,   0, 140,   0,   0,  11, 
     66,   0,  16,   0,   3,   0, 
      0,   0,   1,  64,   0,   0, 
      9,   0,   0,   0,   1,  64, 
      0,   0,   4,   0,   0,   0, 
     42,   0,  16,   0,   3,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  35,   0, 
      0,   9,  66,   0,  16,   0, 
      3,   0,   0,   0,  58,   0, 
     16,   0,   3,   0,   0,   0, 
      1,  64,   0,   0,   0, 128, 
      0,   0,  42,   0,  16,   0, 
      3,   0,   0,   0, 131,   0, 
      0,   5,  66,   0,  16,   0, 
      3,   0,   0,   0,  42,   0, 
     16,   0,   3,   0,   0,   0, 
     56,   0,   0,  10,  50,   0, 
     16,   0,   3,   0,   0,   0, 
    134,   0,  16,   0,   3,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,  71,   0,   0, 
      0,  71,   0,   0,   0,   0, 
      0,   0,   0,   0,  15,   0, 
      0,   7,  18,   0,  16,   0, 
      3,   0,   0,   0,  70,   0, 
     16,   0,   3,   0,   0,   0, 
     70,   0,  16,   0,   3,   0, 
      0,   0,  75,   0,   0,   5, 
     18,   0,  16,   0,   3,   0, 
      0,   0,  10,   0,  16,   0, 
      3,   0,   0,   0,  56,   0, 
      0,   7,  18,   0,  16,   0, 
      3,   0,   0,   0,  10,   0, 
     16,   0,   3,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,  65,  51,   0,   0,   7, 
     18,   0,  16,   0,   3,   0, 
      0,   0,  10,   0,  16,   0, 
      3,   0,   0,   0,   1,  64, 
      0,   0,   0,   0, 128,  63, 
     54,   0,   0,   5,  18,   0, 
     16,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
    128,  63,  50,   0,   0,   9, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      2,   0,   0,   0,   6,   0, 
     16,   0,   3,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,  54,   0,   0,   8, 
    194,   0,  16,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  45,   0, 
      0, 137, 194,   0,   0, 128, 
      3,  17,  17,   0,  18,   0, 
     16,   0,   2,   0,   0,   0, 
     70,  15,  16,   0,   0,   0, 
      0,   0,  70, 126,  16,   0, 
      1,   0,   0,   0,  45,   0, 
      0, 137, 194,   0,   0, 128, 
     67,  85,  21,   0, 226,   0, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,  54, 121,  16,   0, 
      0,   0,   0,   0, 140,   0, 
      0,  11,  34,   0,  16,   0, 
      2,   0,   0,   0,   1,  64, 
      0,   0,   9,   0,   0,   0, 
      1,  64,   0,   0,   4,   0, 
      0,   0,  10,   0,  16,   0, 
      2,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
    138,   0,   0,  15, 210,   0, 
     16,   0,   2,   0,   0,   0, 
      2,  64,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
     10,   0,   0,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
      9,   0,   0,   0,   0,   0, 
      0,   0,  10,   0,   0,   0, 
     19,   0,   0,   0,   6,   0, 
     16,   0,   2,   0,   0,   0, 
     35,   0,   0,   9,  18,   0, 
     16,   0,   2,   0,   0,   0, 
     10,   0,  16,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
      0, 128,   0,   0,  26,   0, 
     16,   0,   2,   0,   0,   0, 
    131,   0,   0,   5,  18,   0, 
     16,   0,   2,   0,   0,   0, 
     10,   0,  16,   0,   2,   0, 
      0,   0, 140,   0,   0,  11, 
     66,   0,  16,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
      9,   0,   0,   0,   1,  64, 
      0,   0,   4,   0,   0,   0, 
     42,   0,  16,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  35,   0, 
      0,   9,  66,   0,  16,   0, 
      2,   0,   0,   0,  58,   0, 
     16,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   0, 128, 
      0,   0,  42,   0,  16,   0, 
      2,   0,   0,   0, 131,   0, 
      0,   5,  66,   0,  16,   0, 
      2,   0,   0,   0,  42,   0, 
     16,   0,   2,   0,   0,   0, 
     56,   0,   0,  10,  50,   0, 
     16,   0,   2,   0,   0,   0, 
    134,   0,  16,   0,   2,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,  71,   0,   0, 
      0,  71,   0,   0,   0,   0, 
      0,   0,   0,   0,  15,   0, 
      0,   7,  18,   0,  16,   0, 
      2,   0,   0,   0,  70,   0, 
     16,   0,   2,   0,   0,   0, 
     70,   0,  16,   0,   2,   0, 
      0,   0,  75,   0,   0,   5, 
     18,   0,  16,   0,   2,   0, 
      0,   0,  10,   0,  16,   0, 
      2,   0,   0,   0,  56,   0, 
      0,   7,  18,   0,  16,   0, 
      2,   0,   0,   0,  10,   0, 
     16,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,  65,  51,   0,   0,   7, 
     18,   0,  16,   0,   2,   0, 
      0,   0,  10,   0,  16,   0, 
      2,   0,   0,   0,   1,  64, 
      0,   0,   0,   0, 128,  63, 
     54,   0,   0,   5,  18,   0, 
     16,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
    128,  63,  50,   0,   0,   9, 
    242,   0,  16,   0,   0,   0, 
      0,   0,  70,  14,  16,   0, 
      0,   0,   0,   0,   6,   0, 
     16,   0,   2,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,  35,   0,   0,  14, 
     50,   0,  16,   0,   1,   0, 
      0,   0,  70,   0,   2,   0, 
      2,  64,   0,   0,   2,   0, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   8, 194,   0,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     45,   0,   0, 137, 194,   0, 
      0, 128,   3,  17,  17,   0, 
     18,   0,  16,   0,   2,   0, 
      0,   0,  70,  15,  16,   0, 
      1,   0,   0,   0,  70, 126, 
     16,   0,   1,   0,   0,   0, 
     45,   0,   0, 137, 194,   0, 
      0, 128,  67,  85,  21,   0, 
    226,   0,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  54, 121, 
     16,   0,   0,   0,   0,   0, 
    140,   0,   0,  11,  34,   0, 
     16,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   9,   0, 
      0,   0,   1,  64,   0,   0, 
      4,   0,   0,   0,  10,   0, 
     16,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0, 138,   0,   0,  15, 
    210,   0,  16,   0,   2,   0, 
      0,   0,   2,  64,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,  10,   0,   0,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,   9,   0,   0,   0, 
      0,   0,   0,   0,  10,   0, 
      0,   0,  19,   0,   0,   0, 
      6,   0,  16,   0,   2,   0, 
      0,   0,  35,   0,   0,   9, 
     18,   0,  16,   0,   2,   0, 
      0,   0,  10,   0,  16,   0, 
      2,   0,   0,   0,   1,  64, 
      0,   0,   0, 128,   0,   0, 
     26,   0,  16,   0,   2,   0, 
      0,   0, 131,   0,   0,   5, 
     18,   0,  16,   0,   2,   0, 
      0,   0,  10,   0,  16,   0, 
      2,   0,   0,   0, 140,   0, 
      0,  11,  66,   0,  16,   0, 
      2,   0,   0,   0,   1,  64, 
      0,   0,   9,   0,   0,   0, 
      1,  64,   0,   0,   4,   0, 
      0,   0,  42,   0,  16,   0, 
      2,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     35,   0,   0,   9,  66,   0, 
     16,   0,   2,   0,   0,   0, 
     58,   0,  16,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
      0, 128,   0,   0,  42,   0, 
     16,   0,   2,   0,   0,   0, 
    131,   0,   0,   5,  66,   0, 
     16,   0,   2,   0,   0,   0, 
     42,   0,  16,   0,   2,   0, 
      0,   0,  56,   0,   0,  10, 
     50,   0,  16,   0,   2,   0, 
      0,   0, 134,   0,  16,   0, 
      2,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,  71, 
      0,   0,   0,  71,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     15,   0,   0,   7,  18,   0, 
     16,   0,   2,   0,   0,   0, 
     70,   0,  16,   0,   2,   0, 
      0,   0,  70,   0,  16,   0, 
      2,   0,   0,   0,  75,   0, 
      0,   5,  18,   0,  16,   0, 
      2,   0,   0,   0,  10,   0, 
     16,   0,   2,   0,   0,   0, 
     56,   0,   0,   7,  18,   0, 
     16,   0,   2,   0,   0,   0, 
     10,   0,  16,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,  65,  51,   0, 
      0,   7,  18,   0,  16,   0, 
      2,   0,   0,   0,  10,   0, 
     16,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
    128,  63,  54,   0,   0,   5, 
     18,   0,  16,   0,   1,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0, 128,  63,  50,   0, 
      0,   9, 242,   0,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
      6,   0,  16,   0,   2,   0, 
      0,   0,  70,  14,  16,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   7,  18,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,  23, 183, 
    209,  56,  14,   0,   0,   7, 
    114,   0,  16,   0,   1,   0, 
      0,   0, 150,   7,  16,   0, 
      0,   0,   0,   0,   6,   0, 
     16,   0,   0,   0,   0,   0, 
     56,   0,   0,   7,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,  64,  63,  65,   0, 
      0,   5, 130,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     56,   0,   0,  10, 114,   0, 
     16,   0,   0,   0,   0,   0, 
    246,  15,  16,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
    171, 170, 170,  62, 171, 170, 
    170,  62, 171, 170, 170,  62, 
      0,   0,   0,   0,  54,   0, 
      0,   5, 130,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0, 171, 170, 170,  62, 
     56,   0,   0,   7, 242,   0, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0, 164,   0, 
      0,   6, 242, 224,  17,   0, 
      0,   0,   0,   0,  70,   5, 
      2,   0,  70,  14,  16,   0, 
      0,   0,   0,   0,  62,   0, 
      0,   1,  83,  84,  65,  84, 
    148,   0,   0,   0,  81,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  30,   0,   0,   0, 
     11,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   8,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  10,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  82,  84,  83,  48, 
    212,   0,   0,   0,   2,   0, 
      0,   0,   4,   0,   0,   0, 
     24,   0,   0,   0,   1,   0, 
      0,   0, 160,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
     72,   0,   0,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
     84,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     96,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    128,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      4,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0, 104,   0,   0,   0, 
      1,   0,   0,   0,   8,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      1,   0,   0,   0, 136,   0, 
      0,   0,   0,   0,   0,   0, 
      8,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,  21,   0,   0,   0, 
      4,   0,   0,   0,   4,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,  16,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 255, 255, 127, 127, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0
};
