#include "Measurer.h"
#include "Generator.h"
#include "fft.h"

#include <QAudioInput>
#include <QAudioOutput>
#include <QDateTime>
#include <QCoreApplication>
#include <qendian.h>

#include <vector>
#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

Measurer::Measurer(QAudioDeviceInfo input, QAudioDeviceInfo output, MeasureChannel channel, bool debug)
    : input(input), output(output), channel(channel), debug(debug)
{
}

long double CurrentTime()
{
	return QDateTime::currentMSecsSinceEpoch() / 1000.0;
}

float Measurer::Measure(float duration)
{
	QAudioFormat format;
	format.setSampleRate(SAMPLE_RATE);
	format.setChannelCount(2);
	format.setSampleSize(16);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::SignedInt);

	// configure generator of single-tone signal
	auto device = new Generator(format, duration * 1000000, TONE_FREQUENCY, channel == MeasureChannel::Right);
	device->start();
	// begin outputting data to device
	QAudioOutput * audio_output = new QAudioOutput(output, format);
	audio_output->start(device);
	// begin reading of samples from input
	QAudioFormat input_format(format);
	input_format.setChannelCount(1);
	QAudioInput *audio_input = new QAudioInput(input, input_format);
	auto reading_device = audio_input->start();
	auto start = CurrentTime();
	QByteArray input_buffer;
	while (CurrentTime() < start + duration)
	{
		QCoreApplication::processEvents();
		input_buffer += reading_device->readAll();
		usleep(20000);
	}
	audio_input->stop();
	audio_output->stop();
	device->stop();
	auto result = ProcessData(input_buffer);
	delete audio_input;
	delete audio_output;
	delete device;
	return result;
}

float Measurer::ProcessData(QByteArray data)
{
	// convert byte data to sample array
	vector<float> read_samples;
	// note that we fixed format to be little-endian, two-byte, signed
	for (int i = 0; i < data.count(); i += 2)
	{
		int t = qFromLittleEndian<qint16>(data.data() + i);
		read_samples.push_back(t / 32767.0);
	}
	if (debug)
	{
		ostringstream oss;
		for (int i = 0; i < (int)read_samples.size(); i++)
		{
			oss << read_samples[i] << " ";
		}
		cerr << "Samples read: " << oss.str() << "\n";
	}
	// cut off some initial samples because of delays in sound subsystem 
	// (otherwise we can hear some older sound which is not finished playing yet)
	vector<float> samples;
	for (int i = (int)(read_samples.size() * INITIAL_DELAY_PERCENTAGE); i < (int)read_samples.size(); i++)
	{
		samples.push_back(read_samples[i]);
	}
	// determine silence threshold (will consider signal less than SILENCE of maximum as silence)
	float max_sample = abs(samples[0]);
	for (int i = 1; i < (int)samples.size(); i++)
	{
		float q = abs(samples[i]);
		if (q > max_sample) max_sample = q;
	}
	float silence_level = SILENCE_LEVEL * max_sample;
	// cut off starting and ending silence
	int left_pos = 0;
	while (left_pos < (int)samples.size() && abs(samples[left_pos]) < silence_level) left_pos++;
	int right_pos = (int)samples.size() - 1;
	while (right_pos >= 0 && abs(samples[right_pos]) < silence_level) right_pos--;
	int len = right_pos - left_pos + 1;
	int pow = 1, m = 0;
	while (2 * pow <= len)
	{
		pow <<= 1;
		m++;
	}
	Complex *d = new Complex[pow];
	for (int i = 0; i < pow; i++)
	{
		d[i] = Complex(samples[left_pos + i], 0);
	}
	FFT(d, pow, false);
	return ProcessSpectrum (pow, d);
}

float Measurer::ProcessSpectrum(int pow, Complex *d)
{
	if (debug)
	{
		ostringstream oss;
		for (int i = 0; i < pow; i++)
		{
			float amp = (float)sqrt(d[i].real() * d[i].real() + d[i].imag() * d[i].imag());
			oss << amp << " ";
		}
		cerr << "Spectrum: " << oss.str() << "\n";
	}
	// will search for maximum value near our frequency
	// also will 
	float desired = TONE_FREQUENCY * pow / SAMPLE_RATE;
	int find_left = (int)((TONE_FREQUENCY - FREQUENCY_GAP) * pow / SAMPLE_RATE);
	if (find_left < 0) find_left = 0;
	int find_right = (int)((TONE_FREQUENCY + FREQUENCY_GAP) * pow / SAMPLE_RATE);
	if (find_right > pow) find_right = pow;
	float max = -1;
	float avg = 0;
	for (int i = find_left; i < find_right; i++)
	{
		float amp = (float)sqrt(d[i].real() * d[i].real() + d[i].imag() * d[i].imag());
		if (amp > max)
		{
			max = amp;
		}
		avg += amp;
	}
	avg /= (find_right - find_left);
	if (debug)
	{
		cerr << "ProcessSpectrum: desired = " << desired << " avg = " << avg << " max = " << max << "\n";
	}
	if (abs(max / avg) - 1 > PEAK_LEVEL)
	{
		return max / pow;
	}
	return nan("");
}
