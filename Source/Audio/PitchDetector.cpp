#include "PitchDetector.h"

#include <cmath>
#include <algorithm>

// ==========================================================
// CONSTRUCTOR
// ==========================================================

PitchDetector::PitchDetector()
{
}

// ==========================================================
// PREPARE
// ==========================================================

void PitchDetector::prepare(
    double newSampleRate,
    int maximumBlockSize)
{
    juce::ignoreUnused(maximumBlockSize);

    sampleRate =
        newSampleRate > 0.0
            ? newSampleRate
            : 44100.0;

    analysisSize = 2048;

    analysisBuffer.setSize(
        1,
        analysisSize
    );

    analysisBuffer.clear();

    analysisWritePosition = 0;

    reset();
}

// ==========================================================
// RESET
// ==========================================================

void PitchDetector::reset()
{
    analysisWritePosition = 0;

    if (analysisBuffer.getNumSamples() > 0)
        analysisBuffer.clear();

    frequencyHz.store(0.0f);
    midiNote.store(-1.0f);
    cents.store(0.0f);
    confidence.store(0.0f);
    pitchDetected.store(false);

    smoothedFrequency = 0.0f;
    smoothedMidiNote = -1.0f;
    smoothedCents = 0.0f;

    lastValidFrequency = 0.0f;
    lastValidConfidence = 0.0f;

    holdFramesRemaining = 0;

    acquisitionFrequency = 0.0f;
    acquisitionConfidence = 0.0f;
    acquisitionFrames = 0;
}

// ==========================================================
// PROCESS BLOCK
// ==========================================================

void PitchDetector::processBlock(
    const juce::AudioBuffer<float>& buffer)
{
    const int numSamples =
        buffer.getNumSamples();

    const int numChannels =
        buffer.getNumChannels();

    if (numSamples <= 0 ||
        numChannels <= 0 ||
        analysisBuffer.getNumSamples() != analysisSize)
    {
        return;
    }

    for (int sample = 0;
         sample < numSamples;
         ++sample)
    {
        float monoSample = 0.0f;

        for (int channel = 0;
             channel < numChannels;
             ++channel)
        {
            monoSample +=
                buffer.getSample(
                    channel,
                    sample
                );
        }

        monoSample /=
            static_cast<float>(
                numChannels
            );

        analysisBuffer.setSample(
            0,
            analysisWritePosition,
            monoSample
        );

        ++analysisWritePosition;

        if (analysisWritePosition >= analysisSize)
        {
            analysisWritePosition = 0;

            analyse();
        }
    }
}

// ==========================================================
// CORRELATION
// ==========================================================

float PitchDetector::calculateCorrelation(
    const float* samples,
    float mean,
    int lag) const
{
    if (samples == nullptr ||
        lag <= 0 ||
        lag >= analysisSize)
    {
        return 0.0f;
    }

    const int usableSamples =
        analysisSize - lag;

    double correlation = 0.0;
    double energyA = 0.0;
    double energyB = 0.0;

    for (int i = 0;
         i < usableSamples;
         ++i)
    {
        const float a =
            samples[i] - mean;

        const float b =
            samples[i + lag] - mean;

        correlation +=
            static_cast<double>(a)
            *
            static_cast<double>(b);

        energyA +=
            static_cast<double>(a)
            *
            static_cast<double>(a);

        energyB +=
            static_cast<double>(b)
            *
            static_cast<double>(b);
    }

    if (energyA <= 0.0 ||
        energyB <= 0.0)
    {
        return 0.0f;
    }

    const double denominator =
        std::sqrt(
            energyA *
            energyB
        );

    if (denominator <= 0.0)
        return 0.0f;

    return static_cast<float>(
        correlation /
        denominator
    );
}

// ==========================================================
// FIND BEST FUNDAMENTAL
// ==========================================================

int PitchDetector::findBestFundamentalLag(
    const float* samples,
    float mean,
    int minimumLag,
    int maximumLag,
    float& selectedCorrelation) const
{
    selectedCorrelation = 0.0f;

    if (samples == nullptr)
        return -1;

    PitchCandidate candidates[maximumCandidates];

    int candidateCount = 0;

    // ======================================================
    // FIND LOCAL AUTOCORRELATION PEAKS
    // ======================================================

    for (int lag = minimumLag + 1;
         lag < maximumLag - 1;
         ++lag)
    {
        const float previous =
            calculateCorrelation(
                samples,
                mean,
                lag - 1
            );

        const float current =
            calculateCorrelation(
                samples,
                mean,
                lag
            );

        const float next =
            calculateCorrelation(
                samples,
                mean,
                lag + 1
            );

        if (current < minimumConfidence)
            continue;

        if (current < previous ||
            current < next)
        {
            continue;
        }

        PitchCandidate candidate;

        candidate.lag = lag;
        candidate.correlation = current;

        candidate.frequency =
            static_cast<float>(
                sampleRate /
                static_cast<double>(lag)
            );

        candidate.score =
            current;

        // ==================================================
        // INSERT INTO TOP CANDIDATES
        // ==================================================

        if (candidateCount < maximumCandidates)
        {
            candidates[candidateCount] =
                candidate;

            ++candidateCount;
        }
        else
        {
            int weakestIndex = 0;

            for (int i = 1;
                 i < maximumCandidates;
                 ++i)
            {
                if (candidates[i].correlation <
                    candidates[weakestIndex].correlation)
                {
                    weakestIndex = i;
                }
            }

            if (candidate.correlation >
                candidates[weakestIndex].correlation)
            {
                candidates[weakestIndex] =
                    candidate;
            }
        }
    }

    if (candidateCount <= 0)
        return -1;

    // ======================================================
    // SORT STRONGEST FIRST
    // ======================================================

    std::sort(
        candidates,
        candidates + candidateCount,
        [](
            const PitchCandidate& a,
            const PitchCandidate& b)
        {
            return
                a.correlation >
                b.correlation;
        }
    );

    // ======================================================
    // FIRST: LOOK FOR FUNDAMENTAL SUPPORTED BY HARMONICS
    // ======================================================

    int selectedLag = -1;

    float selectedScore = 0.0f;

    for (int i = 0;
         i < candidateCount;
         ++i)
    {
        const auto& candidate =
            candidates[i];

        if (hasHarmonicSupport(
                samples,
                mean,
                candidate.lag,
                candidate.correlation))
        {
            float score =
                candidate.correlation;

            // Prefer lower-frequency candidates
            // when their harmonic support is strong.

            const float frequencyPenalty =
                juce::jlimit(
                    0.0f,
                    0.20f,
                    (candidate.frequency - 150.0f)
                    / 1000.0f
                );

            score -=
                frequencyPenalty;

            if (selectedLag < 0 ||
                score > selectedScore)
            {
                selectedLag =
                    candidate.lag;

                selectedScore =
                    score;

                selectedCorrelation =
                    candidate.correlation;
            }
        }
    }

    if (selectedLag > 0)
        return selectedLag;

    // ======================================================
    // FALLBACK
    // ======================================================

    selectedLag =
        candidates[0].lag;

    selectedCorrelation =
        candidates[0].correlation;

    return selectedLag;
}

// ==========================================================
// HARMONIC SUPPORT
// ==========================================================

bool PitchDetector::hasHarmonicSupport(
    const float* samples,
    float mean,
    int fundamentalLag,
    float fundamentalCorrelation) const
{
    if (samples == nullptr ||
        fundamentalLag <= 0)
    {
        return false;
    }

    // ======================================================
    // The autocorrelation peak at the fundamental period
    // should normally have supporting peaks around:
    //
    // lag / 2
    // lag / 3
    // lag * 2
    //
    // depending on harmonic structure.
    // ======================================================

    float support = 0.0f;

    int supportCount = 0;

    // ------------------------------------------------------
    // Half-period
    // ------------------------------------------------------

    const int halfLag =
        static_cast<int>(
            std::round(
                fundamentalLag * 0.5f
            )
        );

    if (halfLag > 20)
    {
        const float correlation =
            calculateCorrelation(
                samples,
                mean,
                halfLag
            );

        if (correlation >
            fundamentalCorrelation * 0.45f)
        {
            support += correlation;

            ++supportCount;
        }
    }

    // ------------------------------------------------------
    // Third-period
    // ------------------------------------------------------

    const int thirdLag =
        static_cast<int>(
            std::round(
                fundamentalLag / 3.0f
            )
        );

    if (thirdLag > 20)
    {
        const float correlation =
            calculateCorrelation(
                samples,
                mean,
                thirdLag
            );

        if (correlation >
            fundamentalCorrelation * 0.35f)
        {
            support += correlation;

            ++supportCount;
        }
    }

    // ------------------------------------------------------
    // Double-period
    // ------------------------------------------------------

    const int doubleLag =
        fundamentalLag * 2;

    if (doubleLag < analysisSize - 2)
    {
        const float correlation =
            calculateCorrelation(
                samples,
                mean,
                doubleLag
            );

        if (correlation >
            fundamentalCorrelation * 0.55f)
        {
            support += correlation;

            ++supportCount;
        }
    }

    // ------------------------------------------------------
    // A strong candidate by itself can still be accepted.
    // But if another candidate has harmonic support, that
    // candidate gets preference in findBestFundamentalLag().
    // ------------------------------------------------------

    if (supportCount >= 2)
        return true;

    // A very strong fundamental gets a softer requirement.

    if (fundamentalCorrelation >= 0.85f &&
        supportCount >= 1)
    {
        return true;
    }

    return false;
}

// ==========================================================
// BETTER FUNDAMENTAL CANDIDATE
// ==========================================================

bool PitchDetector::isBetterFundamentalCandidate(
    int candidateLag,
    float candidateCorrelation,
    int currentLag,
    float currentCorrelation) const
{
    if (candidateLag <= 0)
        return false;

    if (currentLag <= 0)
        return true;

    // Prefer a longer period when correlation is reasonably
    // close to the stronger harmonic peak.

    const float correlationDifference =
        currentCorrelation -
        candidateCorrelation;

    if (correlationDifference <= 0.08f &&
        candidateLag > currentLag)
    {
        return true;
    }

    return
        candidateCorrelation >
        currentCorrelation;
}

// ==========================================================
// ANALYSE
// ==========================================================

void PitchDetector::analyse()
{
    const float* samples =
        analysisBuffer.getReadPointer(0);

    if (samples == nullptr)
        return;

    // ======================================================
    // RMS
    // ======================================================

    double sumSquares = 0.0;

    for (int i = 0;
         i < analysisSize;
         ++i)
    {
        const double value =
            static_cast<double>(
                samples[i]
            );

        sumSquares +=
            value * value;
    }

    const float rms =
        static_cast<float>(
            std::sqrt(
                sumSquares /
                static_cast<double>(
                    analysisSize
                )
            )
        );

    // ======================================================
    // SILENCE
    // ======================================================

    if (rms < 0.003f)
    {
        acquisitionFrames = 0;
        acquisitionFrequency = 0.0f;
        acquisitionConfidence = 0.0f;

        if (holdFramesRemaining > 0 &&
            lastValidFrequency > 0.0f)
        {
            --holdFramesRemaining;

            holdPreviousPitch(0.0f);
        }
        else
        {
            clearPitch();
        }

        return;
    }

    // ======================================================
    // DC OFFSET
    // ======================================================

    float mean = 0.0f;

    for (int i = 0;
         i < analysisSize;
         ++i)
    {
        mean += samples[i];
    }

    mean /=
        static_cast<float>(
            analysisSize
        );

    // ======================================================
    // PITCH RANGE
    // ======================================================

    const int minimumLag =
        juce::jmax(
            1,
            static_cast<int>(
                sampleRate / 1000.0
            )
        );

    const int maximumLag =
        juce::jmin(
            analysisSize - 4,
            static_cast<int>(
                sampleRate / 70.0
            )
        );

    // ======================================================
    // FUNDAMENTAL SELECTION
    // ======================================================

    float bestCorrelation = 0.0f;

    const int bestLag =
        findBestFundamentalLag(
            samples,
            mean,
            minimumLag,
            maximumLag,
            bestCorrelation
        );

    // ======================================================
    // DIAGNOSTIC
    // ======================================================

    static int diagnosticCounter = 0;

    if (diagnosticCounter < 100)
    {
        
        ++diagnosticCounter;
    }

    // ======================================================
    // NO VALID PITCH
    // ======================================================

    if (bestLag <= 0 ||
        bestCorrelation < minimumConfidence)
    {
        if (holdFramesRemaining > 0 &&
            lastValidFrequency > 0.0f)
        {
            --holdFramesRemaining;

            holdPreviousPitch(
                bestCorrelation
            );
        }
        else
        {
            acquisitionFrames = 0;
            acquisitionFrequency = 0.0f;
            acquisitionConfidence = 0.0f;

            clearPitch();
        }

        return;
    }

    // ======================================================
    // FREQUENCY
    // ======================================================

    float detectedFrequency =
        static_cast<float>(
            sampleRate /
            static_cast<double>(
                bestLag
            )
        );

    if (!std::isfinite(detectedFrequency) ||
        detectedFrequency <= 0.0f)
    {
        clearPitch();
        return;
    }

    // ======================================================
    // INITIAL ACQUISITION
    // ======================================================

    if (lastValidFrequency <= 0.0f)
    {
        acquirePitch(
            detectedFrequency,
            bestCorrelation
        );

        return;
    }

    // ======================================================
    // OCTAVE CORRECTION
    // ======================================================

    detectedFrequency =
        correctOctaveError(
            detectedFrequency
        );

    // ======================================================
    // CONTINUITY
    // ======================================================

    const float pitchJump =
        std::abs(
            12.0f *
            std::log2(
                detectedFrequency /
                lastValidFrequency
            )
        );

    if (pitchJump >
        maximumPitchJumpSemitones)
    {
        
        if (holdFramesRemaining > 0)
        {
            --holdFramesRemaining;

            holdPreviousPitch(
                bestCorrelation
            );
        }
        else
        {
            clearPitch();
        }

        return;
    }

    // ======================================================
    // VALID PITCH
    // ======================================================

    storeValidPitch(
        detectedFrequency,
        bestCorrelation
    );
}

// ==========================================================
// INITIAL ACQUISITION
// ==========================================================

bool PitchDetector::acquirePitch(
    float detectedFrequency,
    float detectedConfidence)
{
    if (detectedFrequency <= 0.0f)
        return false;

    // ======================================================
    // FIRST CANDIDATE
    // ======================================================

    if (acquisitionFrequency <= 0.0f)
    {
        acquisitionFrequency =
            detectedFrequency;

        acquisitionConfidence =
            detectedConfidence;

        acquisitionFrames = 1;

        pitchDetected.store(false);

        confidence.store(
            detectedConfidence
        );

        return false;
    }

    // ======================================================
    // CONSISTENCY
    // ======================================================

    if (isCandidateConsistent(
            detectedFrequency))
    {
        const float candidateLog =
            std::log(
                acquisitionFrequency
            );

        const float detectedLog =
            std::log(
                detectedFrequency
            );

        const float blendedLog =
            candidateLog
            +
            (
                detectedLog
                -
                candidateLog
            )
            *
            0.5f;

        acquisitionFrequency =
            std::exp(
                blendedLog
            );

        acquisitionConfidence =
            juce::jmax(
                acquisitionConfidence,
                detectedConfidence
            );

        ++acquisitionFrames;
    }
    else
    {
        // New candidate. Start again.

        acquisitionFrequency =
            detectedFrequency;

        acquisitionConfidence =
            detectedConfidence;

        acquisitionFrames = 1;

        pitchDetected.store(false);

        confidence.store(
            detectedConfidence
        );

        return false;
    }

    // ======================================================
    // REQUIRE 3 FRAMES
    // ======================================================

    if (acquisitionFrames <
        requiredAcquisitionFrames)
    {
        pitchDetected.store(false);

        confidence.store(
            detectedConfidence
        );

        return false;
    }

    // ======================================================
    // LOCK
    // ======================================================

    
    storeValidPitch(
        acquisitionFrequency,
        acquisitionConfidence
    );

    acquisitionFrequency = 0.0f;
    acquisitionConfidence = 0.0f;
    acquisitionFrames = 0;

    return true;
}

// ==========================================================
// CANDIDATE CONSISTENCY
// ==========================================================

bool PitchDetector::isCandidateConsistent(
    float candidateFrequency) const
{
    if (acquisitionFrequency <= 0.0f ||
        candidateFrequency <= 0.0f)
    {
        return false;
    }

    const float semitoneDifference =
        std::abs(
            12.0f *
            std::log2(
                candidateFrequency /
                acquisitionFrequency
            )
        );

    return
        semitoneDifference <=
        acquisitionToleranceSemitones;
}

// ==========================================================
// OCTAVE CORRECTION
// ==========================================================

float PitchDetector::correctOctaveError(
    float detectedFrequency) const
{
    if (detectedFrequency <= 0.0f)
        return detectedFrequency;

    if (lastValidFrequency <= 0.0f)
        return detectedFrequency;

    float corrected =
        detectedFrequency;

    while (corrected >
           lastValidFrequency * 1.5f)
    {
        corrected *= 0.5f;
    }

    while (corrected <
           lastValidFrequency * 0.6666667f)
    {
        corrected *= 2.0f;
    }

    return corrected;
}

// ==========================================================
// STORE VALID PITCH
// ==========================================================

void PitchDetector::storeValidPitch(
    float detectedFrequency,
    float detectedConfidence)
{
    if (detectedFrequency <= 0.0f)
        return;

    // ======================================================
    // LOG-FREQUENCY SMOOTHING
    // ======================================================

    if (smoothedFrequency <= 0.0f)
    {
        smoothedFrequency =
            detectedFrequency;
    }
    else
    {
        const float currentLog =
            std::log(
                smoothedFrequency
            );

        const float detectedLog =
            std::log(
                detectedFrequency
            );

        const float smoothedLog =
            currentLog
            +
            (
                detectedLog
                -
                currentLog
            )
            *
            pitchSmoothing;

        smoothedFrequency =
            std::exp(
                smoothedLog
            );
    }

    // ======================================================
    // MIDI
    // ======================================================

    const float detectedMidi =
        69.0f
        +
        12.0f
        *
        std::log2(
            smoothedFrequency /
            440.0f
        );

    const float nearestMidi =
        std::round(
            detectedMidi
        );

    const float detectedCents =
        (
            detectedMidi
            -
            nearestMidi
        )
        *
        100.0f;

    smoothedMidiNote =
        detectedMidi;

    smoothedCents =
        detectedCents;

    lastValidFrequency =
        smoothedFrequency;

    lastValidConfidence =
        detectedConfidence;

    frequencyHz.store(
        smoothedFrequency
    );

    midiNote.store(
        smoothedMidiNote
    );

    cents.store(
        smoothedCents
    );

    confidence.store(
        detectedConfidence
    );

    pitchDetected.store(
        true
    );

    holdFramesRemaining =
        pitchHoldFrames;
}

// ==========================================================
// HOLD
// ==========================================================

void PitchDetector::holdPreviousPitch(
    float detectedConfidence)
{
    if (lastValidFrequency <= 0.0f)
    {
        clearPitch();
        return;
    }

    frequencyHz.store(
        lastValidFrequency
    );

    midiNote.store(
        smoothedMidiNote
    );

    cents.store(
        smoothedCents
    );

    confidence.store(
        juce::jmax(
            lastValidConfidence,
            detectedConfidence
        )
    );

    pitchDetected.store(
        true
    );
}

// ==========================================================
// CLEAR
// ==========================================================

void PitchDetector::clearPitch()
{
    frequencyHz.store(0.0f);
    midiNote.store(-1.0f);
    cents.store(0.0f);
    confidence.store(0.0f);
    pitchDetected.store(false);

    smoothedFrequency = 0.0f;
    smoothedMidiNote = -1.0f;
    smoothedCents = 0.0f;

    lastValidFrequency = 0.0f;
    lastValidConfidence = 0.0f;

    holdFramesRemaining = 0;

    acquisitionFrequency = 0.0f;
    acquisitionConfidence = 0.0f;
    acquisitionFrames = 0;
}

// ==========================================================
// GETTERS
// ==========================================================

float PitchDetector::getFrequencyHz() const
{
    return frequencyHz.load();
}

float PitchDetector::getMidiNote() const
{
    return midiNote.load();
}

float PitchDetector::getCents() const
{
    return cents.load();
}

float PitchDetector::getConfidence() const
{
    return confidence.load();
}

bool PitchDetector::isPitchDetected() const
{
    return pitchDetected.load();
}