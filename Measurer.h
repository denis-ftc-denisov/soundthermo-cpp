#ifndef MEASURER_H
#define MEASURER_H

#include <QAudioDeviceInfo>

enum MeasureChannel
{
	Left,
	Right
};

class Measurer
{
	const int TONE_FREQUENCY = 6000;
	const int SAMPLE_RATE = 44100;

	MeasureChannel channel;
	QAudioDeviceInfo input, output;

	float ProcessData(QByteArray data);
public:
	Measurer(QAudioDeviceInfo input, QAudioDeviceInfo output, MeasureChannel channel);
	float Measure(float duration);
};

#endif // MEASURER_H
