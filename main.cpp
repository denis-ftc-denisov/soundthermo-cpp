#include <QCoreApplication>
#include <QCommandLineParser>
#include <QAudioDeviceInfo>

#include <iostream>
#include <cmath>

#include "Measurer.h"

using namespace std;

void ListDevices()
{
	{
		auto devices = QAudioDeviceInfo::availableDevices(QAudio::Mode::AudioOutput);
		cout << "Output devices:\n";
		for (int i = 0; i < devices.count(); i++)
		{
			cout << devices[i].deviceName().toStdString() << "\n";
		}
	}
	{
		auto devices = QAudioDeviceInfo::availableDevices(QAudio::Mode::AudioInput);
		cout << "Input devices:\n";
		for (int i = 0; i < devices.count(); i++)
		{
			cout << devices[i].deviceName().toStdString() << "\n";
		}
	}
}

QAudioDeviceInfo GetInputDevice(QString name)
{
	auto devices = QAudioDeviceInfo::availableDevices(QAudio::Mode::AudioInput);
	for (int i = 0; i < devices.count(); i++)
	{
		if (devices[i].deviceName() == name) return devices[i];
	}
	throw (string)("Unable to find device " + name.toStdString());
}

QAudioDeviceInfo GetOutputDevice(QString name)
{
	auto devices = QAudioDeviceInfo::availableDevices(QAudio::Mode::AudioOutput);
	for (int i = 0; i < devices.count(); i++)
	{
		if (devices[i].deviceName() == name) return devices[i];
	}
	throw (string)("Unable to find device " + name.toStdString());
}

void Measure(QString input_device_name, QString output_device_name, float duration)
{
	auto input = GetInputDevice(input_device_name);
	auto output = GetOutputDevice(output_device_name);
	Measurer left(input, output, MeasureChannel::Left);
	float left_result = left.Measure(duration);
	Measurer right(input, output, MeasureChannel::Right);
	float right_result = right.Measure(duration);
	float k = 2 * right_result;
	float v0v = left_result / k;
	// assume r2 = 10K
	float r1 = 10000 / v0v - 10000;
	float t = 1.0 / (log(r1 / 10000) / 4300 + 1 / 298.0) - 273.0;
	cout.setf(ios::fixed | ios::showpoint);
	cout.precision(4);
	cout << t << "\n";
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setApplicationName("soundthermo-cpp");
	QCoreApplication::setApplicationVersion("0.1");

	QCommandLineParser parser;
	parser.setApplicationDescription("Simple measurement of temperature via sound card");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("command", "Command to execute");


	parser.parse(QCoreApplication::arguments());

	const QStringList args = parser.positionalArguments();
	const QString command = args.isEmpty() ? QString() : args.first();
	if (command == "list")
	{
		parser.clearPositionalArguments();
		ListDevices();
	}
	else if (command == "measure")
	{
		QCommandLineOption input_option(QStringList() << "i" << "input", "Name of device to receive input from", "input");
		QCommandLineOption output_option(QStringList() << "o" << "output", "Name of device to send output to", "output");
		QCommandLineOption duration_option(QStringList() << "d" << "duration", "How many seconds to record (uses twice this time)", "duration");
		parser.addOption(input_option);
		parser.addOption(output_option);
		parser.addOption(duration_option);
		parser.process(a);
		QString input_device_name = parser.value(input_option);
		QString output_device_name = parser.value(output_option);
		float duration = parser.value(duration_option).toFloat();
		Measure(input_device_name, output_device_name, duration);
		return 0;
	}
	else
	{
		parser.process(a);
		parser.showHelp(0);
	}
	return a.exec();
}
