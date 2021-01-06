#ifndef MEASURER_H
#define MEASURER_H

#include <QAudioDeviceInfo>

#include "fft.h"

enum MeasureChannel
{
	Left,
	Right
};

class Measurer
{
	// frequency for generating tone
	const int TONE_FREQUENCY = 1000;
	// frequency window width to search for resulting tone 
	const int FREQUENCY_GAP = 100;
	// our resulting amplitude should differ at least this times from average
	// to be considered as measured signal
	const float PEAK_LEVEL = 1.0;
	//
	const int SAMPLE_RATE = 44100;
	const float SILENCE_LEVEL = 0.3;
	const float INITIAL_DELAY_PERCENTAGE = 0.3;

	MeasureChannel channel;
	QAudioDeviceInfo input, output;
	bool debug;

	float ProcessData(QByteArray data);
	float ProcessSpectrum(int pow, Complex* d);
public:
	Measurer(QAudioDeviceInfo input, QAudioDeviceInfo output, MeasureChannel channel, bool debug);
	float Measure(float duration);
};

#endif // MEASURER_H
