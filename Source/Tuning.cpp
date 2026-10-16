/*
  ==============================================================================

    Tuning.cpp

    Scala formats: https://www.huygens-fokker.org/scala/scl_format.html and
    the keyboard mapping section of https://www.huygens-fokker.org/scala/help.htm

  ==============================================================================
*/

#include "Tuning.h"
#include <cmath>

namespace
{
    int floorDiv (int a, int b) { return (a >= 0 ? a : a - (b - 1)) / b; }

    // The lines that carry data: '!' starts a comment line.
    juce::StringArray dataLines (const juce::String& text, bool keepEmpty)
    {
        juce::StringArray out;
        for (auto line : juce::StringArray::fromLines (text))
        {
            if (line.trimStart().startsWithChar ('!')) continue;
            if (! keepEmpty && line.trim().isEmpty()) continue;
            out.add (line);
        }
        return out;
    }

    juce::String firstToken (const juce::String& line)
    {
        return juce::StringArray::fromTokens (line.trim(), " \t", "")[0];
    }

    bool isInteger (const juce::String& s)
    {
        return s.isNotEmpty() && s.trimCharactersAtStart ("-+").containsOnly ("0123456789");
    }

    // A pitch line: cents when it has a dot, else a ratio (a/b or a).
    bool parsePitch (const juce::String& token, double& cents)
    {
        if (token.containsChar ('.'))
        {
            if (! token.trimCharactersAtStart ("-+").containsOnly ("0123456789.")) return false;
            cents = token.getDoubleValue();
            return true;
        }
        const auto num = token.upToFirstOccurrenceOf ("/", false, false);
        const auto den = token.containsChar ('/') ? token.fromFirstOccurrenceOf ("/", false, false) : juce::String ("1");
        if (! num.containsOnly ("0123456789") || ! den.containsOnly ("0123456789") || num.isEmpty() || den.isEmpty())
            return false;
        const double a = num.getDoubleValue(), b = den.getDoubleValue();
        if (a <= 0 || b <= 0) return false;
        cents = 1200.0 * std::log2 (a / b);
        return true;
    }
}

double Tuning::degreeCents (int degree) const
{
    const int p = floorDiv (degree, notes);
    const int r = degree - p * notes;
    return p * cents.back() + (r == 0 ? 0.0 : cents[(size_t) r - 1]);
}

double Tuning::periodRatio() const
{
    return std::pow (2.0, degreeCents (mapSize > 0 ? octaveDegree : notes) / 1200.0);
}

bool Tuning::hasUnmappedKeys() const
{
    for (int m : mapping)
        if (m < 0) return true;
    return false;
}

int Tuning::mapStartKey() const
{
    const int period = periodKeys();
    return middleKey + floorDiv (refKey - middleKey, period) * period;
}

juce::String Tuning::load (const juce::String& sclText, const juce::String& kbmText)
{
    Tuning t;

    // --- .scl: description, number of notes, one pitch per line
    const auto lines = dataLines (sclText, true);   // the description line may be blank
    if (lines.size() < 2) return "The scale file is empty.";
    t.name = lines[0].trim();
    int next = 1;
    while (next < lines.size() && lines[next].trim().isEmpty()) ++next;
    if (next >= lines.size()) return "The scale file doesn't give a number of notes.";
    const auto countToken = firstToken (lines[next]);
    if (! isInteger (countToken) || countToken.getIntValue() < 1 || countToken.getIntValue() > 1024)
        return "The scale file doesn't give a number of notes.";
    t.notes = countToken.getIntValue();
    ++next;
    for (int i = 0; i < t.notes; i++, next++)
    {
        while (next < lines.size() && lines[next].trim().isEmpty()) ++next;
        if (next >= lines.size())
            return "The scale file lists " + juce::String (i) + " of its " + juce::String (t.notes) + " notes.";
        double c = 0;
        if (! parsePitch (firstToken (lines[next]), c))
            return "Can't read note " + juce::String (i + 1) + " of the scale: \"" + lines[next].trim() + "\".";
        t.cents.push_back (c);
    }
    if (t.cents.back() <= 0)
        return "The scale's last note (its period) must be above the first.";

    // --- .kbm: size, first, last, middle, reference key, its frequency, period degree, mapping
    if (kbmText.trim().isNotEmpty())
    {
        const auto k = dataLines (kbmText, false);
        if (k.size() < 7) return "The keyboard mapping file is incomplete.";
        juce::String v[7];
        for (int i = 0; i < 7; i++) v[i] = firstToken (k[i]);
        for (int i : { 0, 1, 2, 3, 4, 6 })
            if (! isInteger (v[i])) return "Can't read line " + juce::String (i + 1) + " of the keyboard mapping.";
        t.mapSize      = v[0].getIntValue();
        t.firstKey     = v[1].getIntValue();
        t.lastKey      = v[2].getIntValue();
        t.middleKey    = v[3].getIntValue();
        t.refKey       = v[4].getIntValue();
        t.refFreq      = v[5].getDoubleValue();
        t.octaveDegree = v[6].getIntValue();
        if (t.mapSize < 0 || t.mapSize > 127)  return "The keyboard mapping's size must be 0 to 127.";
        for (int key : { t.firstKey, t.lastKey, t.middleKey, t.refKey })
            if (key < 0 || key > 127)          return "The keyboard mapping names a key outside 0 to 127.";
        if (t.refFreq <= 0)                    return "The keyboard mapping's reference frequency must be above 0.";
        if (t.octaveDegree < 0)                return "The keyboard mapping's period degree can't be negative.";
        for (int i = 0; i < t.mapSize; i++)
        {
            // entries left out at the end are unmapped
            const auto e = 7 + i < k.size() ? firstToken (k[7 + i]) : juce::String ("x");
            if (e.equalsIgnoreCase ("x"))      t.mapping.push_back (-1);
            else if (isInteger (e) && e.getIntValue() >= 0) t.mapping.push_back (e.getIntValue());
            else return "Can't read mapping entry " + juce::String (i + 1) + " of the keyboard mapping.";
        }
        if (t.mapSize > 0 && t.octaveDegree == 0)
            return "The keyboard mapping needs the scale degree its pattern repeats at.";
    }

    // --- every key's scale degree, then its frequency from the reference key
    // (the reference key only has to be mapped, not inside first .. last)
    auto degreeOf = [&t] (int key, bool& inMap)
    {
        const int i = key - t.middleKey;
        inMap = true;
        if (t.mapSize == 0) return i;
        const int entry = t.mapping[(size_t) (i - floorDiv (i, t.mapSize) * t.mapSize)];
        inMap = entry >= 0;
        return entry + floorDiv (i, t.mapSize) * t.octaveDegree;
    };
    bool refInMap = false;
    const int refDegree = degreeOf (t.refKey, refInMap);
    if (! refInMap) return "The keyboard mapping's reference key isn't mapped.";
    const double refCents = t.degreeCents (refDegree);

    int anyMapped = -1;
    for (int key = 0; key < 128; key++)
    {
        bool inMap = false;
        const int d = degreeOf (key, inMap);
        const bool ok = inMap && key >= t.firstKey && key <= t.lastKey;
        t.mapped[key] = ok;
        t.freq[key] = ok ? t.refFreq * std::pow (2.0, (t.degreeCents (d) - refCents) / 1200.0) : 0.0;
        if (ok && anyMapped < 0) anyMapped = key;
    }
    if (anyMapped < 0) return "The keyboard mapping leaves every key unmapped.";

    // unmapped keys sound like the nearest mapped key below (above at the bottom)
    double last = t.freq[anyMapped];
    for (int key = 0; key < 128; key++)
    {
        if (t.mapped[key]) last = t.freq[key];
        else               t.freq[key] = last;
    }

    t.scl = sclText;
    t.kbm = kbmText;
    if (t.name.isEmpty()) t.name = juce::String (t.notes) + "-note scale";
    t.sclFile = sclFile;
    t.kbmFile = kbmFile;
    *this = t;
    return {};
}
