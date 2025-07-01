#pragma once
// 世界標準4オクターブを使用(小数点第二を整数で登録)
enum class GermanNote {
    C = 26163,      // C
    Cis = 27718,    // C# (ツィス)
    D = 29366,      // D
    Dis = 31113,    // D# (ディス)
    E = 32963,      // E
    F = 34923,      // F
    Fis = 36999,    // F# (フィス)
    G = 39200,      // G
    Gis = 41530,    // G# (ギス)
    A = 44000,      // A
    Ais = 46616,    // A# (アイス)
    B = 49388       // H (ハー) - 国際表記のB
};

float GetFrequencyFormGermanNote(GermanNote note, int octave);