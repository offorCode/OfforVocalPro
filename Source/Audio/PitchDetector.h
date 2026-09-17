#pragma once

#include <JuceHeader.h>

class PitchDetector
{
public:

    PitchDetector();

    void prepare(
        double sampleRate,
        int maximumBlockSize
    );

    void reset();

    void processBlock(
        const juce::AudioBuffer<float>& buffer
    );

    float getFrequencyHz() const;
    float getMidiNote() const;
    float getCents() const;
    float getConfidence() const;
    bool isPitchDetected() const;

private:

    double sampleRate = 44100.0;

    int analysisSize = 2048;

    juce::AudioBuffer<float> analysisBuffer;

    int analysisWritePosition = 0;

    std::atomic<float> frequencyHz { 0.0f };
    std::atomic<float> midiNote { -1.0f };
    std::atomic<float> cents { 0.0f };
    std::atomic<float> confidence { 0.0f };
    std::atomic<bool> pitchDetected { false };

    // ==========================================================
    // SMOOTHED / TRACKED PITCH
    // ==========================================================

    float smoothedFrequency = 0.0f;
    float smoothedMidiNote = -1.0f;
    float smoothedCents = 0.0f;

    float lastValidFrequency = 0.0f;
    float lastValidConfidence = 0.0f;

    // ==========================================================
    // PITCH HOLD
    // ==========================================================

    int holdFramesRemaining = 0;

    static constexpr int pitchHoldFrames = 4;

    // ==========================================================
    // PITCH SMOOTHING
    // ==========================================================

    static constexpr float pitchSmoothing = 0.25f;

    // ==========================================================
    // CONFIDENCE
    // ==========================================================

    static constexpr float minimumConfidence = 0.60f;

    // ==========================================================
    // INITIAL ACQUISITION
    // ==========================================================

    float acquisitionFrequency = 0.0f;
    float acquisitionConfidence = 0.0f;

    int acquisitionFrames = 0;

    static constexpr int requiredAcquisitionFrames = 3;

    static constexpr float acquisitionToleranceSemitones = 2.5f;

    // ==========================================================
    // TRACKING
    // ==========================================================

    static constexpr float maximumPitchJumpSemitones = 7.0f;

    // ==========================================================
    // AUTOCORRELATION CANDIDATES
    // ==========================================================

    static constexpr int maximumCandidates = 12;

    struct PitchCandidate
    {
        int lag = 0;
        float correlation = 0.0f;
        float frequency = 0.0f;
        float score = 0.0f;
    };

    // ==========================================================
    // ANALYSIS
    // ==========================================================

    void analyse();

    float calculateCorrelation(
        const float* samples,
        float mean,
        int lag
    ) const;

    int findBestFundamentalLag(
        const float* samples,
        float mean,
        int minimumLag,
        int maximumLag,
        float& selectedCorrelation
    ) const;

    bool hasHarmonicSupport(
        const float* samples,
        float mean,
        int fundamentalLag,
        float fundamentalCorrelation
    ) const;

    bool isBetterFundamentalCandidate(
        int candidateLag,
        float candidateCorrelation,
        int currentLag,
        float currentCorrelation
    ) const;

    float correctOctaveError(
        float detectedFrequency
    ) const;

    bool isCandidateConsistent(
        float candidateFrequency
    ) const;

    bool acquirePitch(
        float detectedFrequency,
        float detectedConfidence
    );

    void storeValidPitch(
        float detectedFrequency,
        float detectedConfidence
    );

    void holdPreviousPitch(
        float detectedConfidence
    );

    void clearPitch();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        PitchDetector
    );
};